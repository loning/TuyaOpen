#include "c_utils/c_utils.h"
#include "lib_c_storage.h"
#include "lib_c_sdk.h"
#include "c_utils/hal_util_time.h"
#include "http_client_interface.h"
#include "tuya_cloud_types.h"
#include "tal_memory.h"
#include "tuya_iot.h"

#include <stdio.h>
#include <string.h>
#define C_SDK_REQ_LEN_REGISTER 500
char req[C_SDK_REQ_LEN_REGISTER];

/* ---- Aliyun service endpoints ---- */
#define METHOD       "POST"
#define HOST         "bailian.multimodalagent.aliyuncs.com"
#define REGISTER_API "/api/device/v1/register"
#define TOKEN_API    "/api/token/v1/getToken"
#define PORT         80
#define HEADER       ""

#define ALIYUN_APP_SECRET "04ec6ee04fbb32cca729f2c86e563b45"
#define ALIYUN_API_KEY    "sk-37341dcc8c5741afb2b7cc07ff77c6e6"
#define ALIYUN_APP_NAME   "多模态应用-TuyaOpen"
#define ALIYUN_APP_ID     "mm_061f6d6cee864c68941bf049d881"

int32_t _mmi_event_callback(uint32_t event, void *param);

/**
 * @brief Makes an HTTP/HTTPS request using TuyaOpen SDK
 *
 * @param method HTTP method (GET, POST, etc.)
 * @param host Target hostname
 * @param api API endpoint path
 * @param port Target port (443 for HTTPS, 80 for HTTP)
 * @param header Additional headers (can be NULL)
 * @param body Request body (can be NULL)
 * @param response_cb Callback function to handle response
 * @return 0 on success, error code on failure
 */
static int aliyun_https_request(const char *method, const char *host, const char *api, int port, const char *header,
                                const char *body, int (*response_cb)(int status, char *rsp_data, int rsp_len))
{
    int rt = OPRT_OK;

    /* HTTP Response */
    http_client_response_t http_response = {0};

    /* HTTP headers */
    http_client_header_t headers[3];
    uint8_t headers_count = 0;

    // Add Accept header
    headers[headers_count].key = "Accept";
    headers[headers_count].value = "*/*";
    headers_count++;

    // Add Content-Type header
    headers[headers_count].key = "Content-Type";
    headers[headers_count].value = "application/json";
    headers_count++;

    // Add Authorization header if provided
    if (header && strlen(header) > 0) {
        char *colon_pos = strchr(header, ':');
        if (colon_pos && headers_count < 3) {
            headers[headers_count].key = "Authorization";
            headers[headers_count].value = colon_pos + 1;
            // Skip leading whitespace in value
            while (*headers[headers_count].value == ' ') {
                headers[headers_count].value++;
            }
            headers_count++;
        }
    }

    /* HTTP Request send */
    PR_DEBUG("HTTP request send! Method: %s, Host: %s, API: %s, Port: %d\n", method, host, api, port);
    PR_DEBUG("HTTP headers count: %d\n", headers_count);
    for (int i = 0; i < headers_count; i++) {
        PR_DEBUG("  Header %d: %s: %s\n", i, headers[i].key, headers[i].value);
    }
    if (body && strlen(body) > 0) {
        PR_DEBUG("HTTP request body: \n%.*s\n", (int)strlen(body), body);
    } else {
        PR_DEBUG("HTTP request body: (empty)\n");
    }

    http_client_status_t http_status =
        http_client_request(&(const http_client_request_t){.host = host,
                                                           .port = (uint16_t)port,
                                                           .method = method,
                                                           .path = api,
                                                           .headers = headers,
                                                           .headers_count = headers_count,
                                                           .body = (const uint8_t *)body,
                                                           .body_length = body ? strlen(body) : 0,
                                                           .timeout_ms = 60000},
                            &http_response);

    if (HTTP_CLIENT_SUCCESS != http_status) {
        PR_DEBUG("http_request_send error:%d\n", http_status);
        rt = OPRT_LINK_CORE_HTTP_CLIENT_SEND_ERROR;
        goto err_exit;
    }

    PR_DEBUG("HTTP response - Status: %d, Body Length: %zu\n", http_response.status_code, http_response.body_length);

    if (http_response.body && http_response.body_length > 0) {
        PR_DEBUG("HTTP response body: \n%.*s\n", (int)http_response.body_length, (char *)http_response.body);

        // Call response callback if provided
        if (response_cb) {
            // Null-terminate the response body for callback
            char *response_str = tal_malloc(http_response.body_length + 1);
            if (response_str) {
                memcpy(response_str, http_response.body, http_response.body_length);
                response_str[http_response.body_length] = '\0';

                rt = response_cb(http_response.status_code, response_str, http_response.body_length);
                tal_free(response_str);
            } else {
                PR_DEBUG("Failed to allocate memory for response string\n");
                rt = OPRT_MALLOC_FAILED;
            }
        }
    } else if (response_cb) {
        // Call callback with empty response
        rt = response_cb(http_response.status_code, NULL, 0);
    }

err_exit:
    http_client_free(&http_response);

    return rt;
}

static int dummy_wss_connect(const char *host, const char *port, const char *api, const char *header)
{
    (void)host;
    (void)port;
    (void)api;
    (void)header;
    return 0;
}

static void dummy_wss_recv(int *opcode, uint8_t **payload, int *recv_size)
{
    if (opcode) {
        *opcode = 0;
    }
    if (payload) {
        *payload = NULL;
    }
    if (recv_size) {
        *recv_size = 0;
    }
}

static int dummy_wss_send(uint8_t data_type, const uint8_t *data, uint32_t size)
{
    (void)data_type;
    (void)data;
    (void)size;
    return 0;
}

static int dummy_recoder_is_work(void)
{
    return 0;
}
static uint32_t dummy_hw_recorder_get_data(uint8_t *data, uint32_t size)
{
    (void)data;
    (void)size;
    return 0;
}
static int dummy_player_is_work(void)
{
    return 0;
}
static int dummy_hw_player_put_data(const uint8_t *data, uint32_t size)
{
    (void)data;
    (void)size;
    return 0;
}

/**
 * @brief Handles HTTP response for device registration
 *
 * @param status HTTP status code
 * @param rsp_data Response data from server
 * @param rsp_len Length of response data
 * @return 0 on success, error code on failure
 */
int aliyun_register_response_handler(int status, char *rsp_data, int rsp_len)
{
    int err;

    PR_DEBUG("ALIYUN aliyun_register_response_handler");

    // ... 客户其他业务逻辑

    // 解析并完成注册
    // SDK中的解析函数只需要报文中的data字段，如下http接收报文所示
    err = c_device_analyze_register_rsp(rsp_data);

    // ... 客户其他业务逻辑

    return err;
}

/**
 * @brief Handles HTTP response for token acquisition
 *
 * @param status HTTP status code
 * @param rsp_data Response data from server
 * @param rsp_len Length of response data
 * @return 0 on success, error code on failure
 */
int aliyun_token_response_handler(int status, char *rsp_data, int rsp_len)
{
    int err;
    PR_DEBUG("ALIYUN aliyun_token_response_handler");

    // ... 客户其他业务逻辑

    // 解析并获取token，数据要求同上
    err = c_mmi_analyze_get_token_rsp(rsp_data);
    // ... 客户其他业务逻辑

    return err;
}

/**
 * @brief Initializes the Aliyun SDK with proper HTTPS communication
 *
 * @return 0 on success, error code on failure
 */
int aliyun_sdk_initialize(void)
{
    // Clear any corrupted storage data first
    PR_DEBUG("ALIYUN clearing storage to fix buffer size issues...");
    c_storage_reset();

    // 初始化SDK
    c_mmi_sdk_init();

    // 设备注册，在此之前需要完成SDK的初始化
    if (c_storage_device_is_registered() == 0) {
        // 配置设备信息
        c_storage_set_app_id_str(ALIYUN_APP_ID);
        c_storage_set_app_secret_str(ALIYUN_APP_SECRET);
        c_storage_set_device_name("EXAMPLE_DEVICE_NAME");
        // 根据当前时间戳timestamp（毫秒），生成注册信息字串req
        int64_t ts = util_get_timestamp_ms();
        char ts_str[32];
        snprintf(ts_str, sizeof(ts_str), "%lld", (long long)ts);
        c_device_gen_register_req(req, ts_str);
        // 获取服务端返回设备信息
        aliyun_https_request(METHOD, HOST, REGISTER_API, PORT, HEADER, req, aliyun_register_response_handler);
    }

    // 设备登录
    if (c_mmi_is_token_expire() == 0) {
        // 根据当前时间戳timestamp（毫秒），生成登录信息字串req
        int64_t ts_login = util_get_timestamp_ms();
        char ts_login_str[32];
        snprintf(ts_login_str, sizeof(ts_login_str), "%lld", (long long)ts_login);
        c_mmi_gen_get_token_req(req, ts_login_str);
        // 获取服务端返回登录信息
        aliyun_https_request(METHOD, HOST, TOKEN_API, PORT, HEADER, req, aliyun_token_response_handler);
    }

    // 初始化音频配置
    mmi_user_config_t mmi_config = {
        .evt_cb = _mmi_event_callback, // 注册事件回调函数，详细说明见下文

        .work_mode = C_MMI_MODE_PUSH2TALK, // PUSH TO TALK模式
        .text_mode = C_MMI_TEXT_MODE_BOTH, // 同时显示ASR文本和LLM文本
        .voice_id = "longxiaochun_v2",

        .upstream_mode = C_MMI_STREAM_MODE_PCM,   // 上行PCM数据
        .downstream_mode = C_MMI_STREAM_MODE_PCM, // 下行PCM数据

        .recorder_rb_size = 8 * 1024, // 录音数据缓冲区大小
        .player_rb_size = 8 * 1024,   // 音频数据缓冲区大小
    };
    c_mmi_config(&mmi_config);

    return 0;
}

int dummy_wss_init(void)
{
    // 建立websocket连接，获取连接字段
    char *wss_host = c_mmi_get_wss_host();
    char *wss_port = c_mmi_get_wss_port();
    char *wss_api = c_mmi_get_wss_api();
    char *wss_header = c_mmi_get_wss_header();

    // 建立wss连接
    int ret = dummy_wss_connect(wss_host, wss_port, wss_api, wss_header);

    return ret;
}

void dummy_recorder_task(void)
{
    uint32_t send_size = 0;
    uint32_t size = 640;
    uint8_t *data = (uint8_t *)util_malloc(size);

    while (1) {
        if (dummy_recoder_is_work()) {
            send_size = dummy_hw_recorder_get_data(data, size);
            if (send_size) {
                // 将音频采集硬件采集到的数据输出至SDK ringbuffer
                c_mmi_put_recorder_data(data, send_size);
            } else {
                util_msleep(10);
            }

        } else {
            util_msleep(10);
        }
    }
}

void dummy_player_task(void)
{
    uint8_t data[640];
    uint32_t size = 640;
    uint32_t recv_size = 0;

    while (1) {
        if (dummy_player_is_work()) {
            recv_size = c_mmi_get_player_data(data, size);
            if (recv_size) {
                // 将SDK ringbuffer中的音频数据输出给到播放器进行播放
                dummy_hw_player_put_data(data, size);
            } else {
                util_msleep(10);
            }

        } else {
            util_msleep(10);
        }
    }
}

int dummy_wss_task_recv(void)
{
    int opcode;
    uint8_t *payload_data;
    int recv_size;
    while (1) {
        dummy_wss_recv(&opcode, &payload_data, &recv_size);
        if (recv_size)
            // 将收到的websocket数据送入SDK进行解析，其中只需要送入opcode，payload数据.
            c_mmi_analyze_recv_data(opcode, payload_data, recv_size);
        else
            util_msleep(10);
    }
    return 0;
}

int dummy_wss_task_send(void)
{
    uint8_t data_type;
    uint8_t payload_data[1024];
    uint32_t size;

    while (1) {
        // 从SDK获取打包好的payload数据
        size = c_mmi_get_send_data(&data_type, payload_data, sizeof(payload_data));
        if (size == 0) {
            util_msleep(10);
        } else {
            // 将payload数据传入websocket发送函数，自行打包帧头进行发送
            dummy_wss_send(data_type, payload_data, size);
        }
    }
    return 0;
}

int dummy_button_up(void)
{
    // 关闭麦克风
    // recorder_stop();
    // 通知云端服务开始处理音频数据
    c_mmi_stop_speech();

    return 0;
}

int dummy_button_down(void)
{
    // 关闭喇叭
    // player_stop();
    // 通知云端即将开始发送音频数据，SDK会根据云端对该指令的响应触发C_MMI_EVENT_SPEECH_START
    c_mmi_start_speech();

    return 0;
}

int32_t _mmi_event_callback(uint32_t event, void *param)
{
    char *text;

    text = param;
    switch (event) {
    case C_MMI_EVENT_USER_CONFIG:
        // 开始新对话
        // c_mmi_reset_dialog_id();
        break;
    case C_MMI_EVENT_DATA_INIT:
        // Mmi data ready, 开始网络连接
        dummy_wss_init();
        break;
    case C_MMI_EVENT_DATA_DEINIT:
        UTIL_LOG_W("will disconnect");
        break;
    case C_MMI_EVENT_SPEECH_START:
        UTIL_LOG_D("enable recorder when send speech");
        // dummy_player_stop();
        // dummy_recorder_start();
        break;
    case C_MMI_EVENT_ASR_START:
        UTIL_LOG_I("event [C_MMI_EVENT_ASR_START]");
        break;
    case C_MMI_EVENT_ASR_INCOMPLETE:
        UTIL_LOG_D("ASR [%s]", text);
        break;
    case C_MMI_EVENT_ASR_COMPLETE:
        if (text) {
            UTIL_LOG_D("ASR C [%s]", text);
        } else {
            UTIL_LOG_D("ASR C [NULL]");
        }
        break;
    case C_MMI_EVENT_ASR_END:
        UTIL_LOG_D("disable record when ASR complete");
        // dummy_recorder_stop();
        break;
    case C_MMI_EVENT_LLM_INCOMPLETE:
        UTIL_LOG_D("LLM [%s]", text);
        break;
    case C_MMI_EVENT_LLM_COMPLETE:
        UTIL_LOG_D("LLM C [%s]", text);
        break;
    case C_MMI_EVENT_TTS_START:
        UTIL_LOG_I("enable player when dialog start");
        // dummy_player_start();
        break;
    case C_MMI_EVENT_TTS_END:
        break;
    default:
        break;
    }

    return UTIL_SUCCESS;
}

void aliyun_sdk_init(void)
{
    aliyun_sdk_initialize();
}

void aliyun_sdk_start(void) {}