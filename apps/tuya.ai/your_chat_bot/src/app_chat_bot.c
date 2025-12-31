/**
 * @file app_chat_bot.c
 * @brief app_chat_bot module is used to
 * @version 0.1
 * @date 2025-03-25
 */

#include "tuya_cloud_types.h"
#include "tal_api.h"

#include "ai_chat_main.h"
#include "ai_mcp.h"

#if defined(ENABLE_COMP_AI_VIDEO) && (ENABLE_COMP_AI_VIDEO == 1)
#include "ai_video_input.h"
#endif

#include "app_display.h"
#include "app_chat_bot.h"
/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    AI_CHAT_MODE_E mode;
    char *display_text;
} CHAT_WORK_MODE_INFO_T;

typedef struct {
    uint8_t is_enable;
    const CHAT_WORK_MODE_INFO_T *work;
} APP_CHAT_BOT_S;

/***********************************************************
***********************const declaration********************
***********************************************************/
const CHAT_WORK_MODE_INFO_T cAPP_WORK_HOLD = {
    .mode = AI_CHAT_MODE_HOLD,
    .display_text = HOLD_TALK,
};

const CHAT_WORK_MODE_INFO_T cAPP_WORK_ONE_SHOT = {
    .mode = AI_CHAT_MODE_ONE_SHOT,
    .display_text = TRIG_TALK,
};

const CHAT_WORK_MODE_INFO_T cAPP_WORK_WAKEUP = {
    .mode = AI_CHAT_MODE_WAKEUP,
    .display_text = WAKEUP_TALK,
};

const CHAT_WORK_MODE_INFO_T cAPP_WORK_FREE = {
    .mode = AI_CHAT_MODE_FREE,
    .display_text = FREE_TALK,
};

const CHAT_WORK_MODE_INFO_T *cWORK_MODE_INFO_LIST[] = {
    &cAPP_WORK_HOLD,
    &cAPP_WORK_ONE_SHOT,
    &cAPP_WORK_WAKEUP,
    &cAPP_WORK_FREE,
};

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/
#if (defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)) || (defined(ENABLE_CHAT_DISPLAY2) && (ENABLE_CHAT_DISPLAY2 == 1))
static void __ai_chat_handle_state(AI_MODE_STATE_E state)
{
    switch (state) {
    case AI_MODE_STATE_INIT:
    case AI_MODE_STATE_IDLE:
        app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t *)EMOJI_NEUTRAL, strlen(EMOJI_NEUTRAL));
        app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)STANDBY, strlen(STANDBY));
        break;
    case AI_MODE_STATE_LISTEN:
        app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)LISTENING, strlen(LISTENING));
        break;
    case AI_MODE_STATE_SPEAK:
        app_display_send_msg(TY_DISPLAY_TP_STATUS, (uint8_t *)SPEAKING, strlen(SPEAKING));
        break;
    default:
        break;
    }
}

static void __ai_chat_handle_event(AI_NOTIFY_EVENT_T *event)
{
    AI_NOTIFY_TEXT_T *text = NULL;

    if(NULL == event) {
        return;
    }

    if (event->type != AI_USER_EVT_MIC_DATA) {
        PR_DEBUG("ai chat event type: %d", event->type);
    }

    switch(event->type) {
    case AI_USER_EVT_ASR_OK:{
        text = (AI_NOTIFY_TEXT_T *)event->data;

        if(text && text->datalen > 0 && text->data) {
            app_display_send_msg(TY_DISPLAY_TP_USER_MSG, (uint8_t *)text->data, text->datalen);
        }
    }
    break;
    case AI_USER_EVT_TEXT_STREAM_START:{
        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_START, NULL, 0);

        text = (AI_NOTIFY_TEXT_T *)event->data;
        if(text && text->datalen > 0 && text->data) {
            app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_DATA, (uint8_t *)text->data, text->datalen);
        }
    }
    break;
    case AI_USER_EVT_TEXT_STREAM_DATA:{
        text = (AI_NOTIFY_TEXT_T *)event->data;
        if(text && text->datalen > 0 && text->data) {
            app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_DATA, (uint8_t *)text->data, text->datalen);
        }
    }
    break;
    case AI_USER_EVT_TEXT_STREAM_STOP:{
        text = (AI_NOTIFY_TEXT_T *)event->data;
        if(text && text->datalen > 0 && text->data) {
            app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_DATA, (uint8_t *)text->data, text->datalen);
        }

        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_END, NULL, 0);
    }
    break;
    case AI_USER_EVT_CHAT_BREAK:{
        app_display_send_msg(TY_DISPLAY_TP_ASSISTANT_MSG_STREAM_INTERRUPT, NULL, 0);
    }
    break;
    case AI_USER_EVT_LLM_EMOTION:
    case AI_USER_EVT_EMOTION:{
        AI_NOTIFY_EMO_T *emo = (AI_NOTIFY_EMO_T *)(event->data);

        for (uint32_t idx = 0; idx < emo->emo_cnt; idx++) {
            app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t *)emo->text[idx], strlen(emo->text[idx]));
        }
    }
    break;
    case AI_USER_EVT_MODE_STATE_UPDATE:{
        AI_MODE_STATE_E state = (AI_MODE_STATE_E )(event->data);
        __ai_chat_handle_state(state);
    }
    break;
    case AI_USER_EVT_MODE_SWITCH:{
        AI_CHAT_MODE_E mode = (AI_CHAT_MODE_E)(event->data);

        if (mode >= CNTSOF(cWORK_MODE_INFO_LIST)) {
            break;
        }

        app_display_send_msg(TY_DISPLAY_TP_CHAT_MODE, \
                            (uint8_t *)cWORK_MODE_INFO_LIST[mode]->display_text,\
                             strlen(cWORK_MODE_INFO_LIST[mode]->display_text));
    }
    break;
    case AI_USER_EVT_VEDIO_DISPLAY_START:{
        AI_NOTIFY_VIDEO_START_T *video_start = (AI_NOTIFY_VIDEO_START_T *)(event->data);
        if(NULL == video_start) {
            PR_ERR("video start param is null");
            break;
        }

        app_display_camera_start(video_start->camera_width, video_start->camera_height);
    }
    break;
    case AI_USER_EVT_VEDIO_DISPLAY_END:
        app_display_camera_end();
    break;
    case AI_USER_EVT_MIC_DATA:{
        #if defined(ENABLE_CHAT_DISPLAY2) && (ENABLE_CHAT_DISPLAY2 == 1)
        AI_NOTIFY_MIC_DATA_T *mic_data = (AI_NOTIFY_MIC_DATA_T *)(event->data);
        if(NULL == mic_data) {
            PR_ERR("mic data is null");
            break;
        }
        extern void app_ui_helper_calculate_audio_power(uint8_t *audio_data, uint32_t data_len);
        static uint32_t calculate_power_tick = 0;
        calculate_power_tick++;
        if (calculate_power_tick % 10 == 0) {
            app_ui_helper_calculate_audio_power(mic_data->data, mic_data->data_len);
        }
        #endif
    }
    break;
    default:
        break;
    }
}

#else
static void __ai_chat_handle_event(AI_NOTIFY_EVENT_T *event)
{

}
#endif

#if defined(ENABLE_COMP_AI_VIDEO) && (ENABLE_COMP_AI_VIDEO == 1)
void ai_video_display_flush(TDL_CAMERA_FRAME_T *frame)
{
    #if defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)
    app_display_camera_flush(frame->data, frame->width, frame->height);
    #endif
}
#endif

OPERATE_RET app_chat_bot_init(void)
{
    OPERATE_RET rt = OPRT_OK;

#if (defined(ENABLE_CHAT_DISPLAY) && (ENABLE_CHAT_DISPLAY == 1)) || ((defined(ENABLE_CHAT_DISPLAY2) && (ENABLE_CHAT_DISPLAY2 == 1)))
    TUYA_CALL_ERR_LOG(app_display_init());
#endif

#if defined(ENABLE_COMP_AI_VIDEO) && (ENABLE_COMP_AI_VIDEO == 1)
    AI_VEDIO_CFG_T ai_video_cfg = {
        .disp_flush_cb = ai_video_display_flush,
    };

    TUYA_CALL_ERR_LOG(ai_video_init(&ai_video_cfg));
#endif

    TUYA_CALL_ERR_RETURN(ai_mode_hold_register());
    TUYA_CALL_ERR_RETURN(ai_mode_oneshot_register());
    TUYA_CALL_ERR_RETURN(ai_mode_wakeup_register());
    TUYA_CALL_ERR_RETURN(ai_mode_free_register());

    AI_CHAT_MODE_CFG_T ai_chat_cfg = {
        .evt_cb = __ai_chat_handle_event,
    };
    TUYA_CALL_ERR_RETURN(ai_chat_init(&ai_chat_cfg));

    TUYA_CALL_ERR_RETURN(ai_mcp_init());

    return OPRT_OK;
}

