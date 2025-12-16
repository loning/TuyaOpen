/**
 * @file tuya_t5ai_eink_nfc.c
 * @author Tuya Inc.
 * @brief Implementation of common board-level hardware registration APIs for audio, button, and SD card peripherals.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#include "tuya_cloud_types.h"

#include "tkl_pinmux.h"
#include "tal_api.h"
#include "tdd_audio.h"
#include "tdd_led_gpio.h"
#include "tdd_button_gpio.h"
#include "board_power_domain_api.h"
#include "board_charge_detect_api.h"
#include "tkl_fs.h"

/***********************************************************
************************macro define************************
***********************************************************/
#define BOARD_SPEAKER_EN_PIN TUYA_GPIO_NUM_42

// Button name definitions
#define BOARD_BUTTON_NAME_UP     "btn_up"
#define BOARD_BUTTON_NAME_DOWN   "btn_down"
#define BOARD_BUTTON_NAME_ENTER  "btn_enter"
#define BOARD_BUTTON_NAME_RETURN "btn_return"
#define BOARD_BUTTON_NAME_LEFT   "btn_left"
#define BOARD_BUTTON_NAME_RIGHT  "btn_right"

#define BOARD_LED_NAME "user_led"

// Button pin definitions
#define BOARD_BUTTON_UP_PIN           TUYA_GPIO_NUM_22
#define BOARD_BUTTON_UP_ACTIVE_LV     TUYA_GPIO_LEVEL_LOW
#define BOARD_BUTTON_DOWN_PIN         TUYA_GPIO_NUM_23
#define BOARD_BUTTON_DOWN_ACTIVE_LV   TUYA_GPIO_LEVEL_LOW
#define BOARD_BUTTON_ENTER_PIN        TUYA_GPIO_NUM_24
#define BOARD_BUTTON_ENTER_ACTIVE_LV  TUYA_GPIO_LEVEL_LOW
#define BOARD_BUTTON_RETURN_PIN       TUYA_GPIO_NUM_25
#define BOARD_BUTTON_RETURN_ACTIVE_LV TUYA_GPIO_LEVEL_LOW
#define BOARD_BUTTON_LEFT_PIN         TUYA_GPIO_NUM_26
#define BOARD_BUTTON_LEFT_ACTIVE_LV   TUYA_GPIO_LEVEL_LOW
#define BOARD_BUTTON_RIGHT_PIN        TUYA_GPIO_NUM_28
#define BOARD_BUTTON_RIGHT_ACTIVE_LV  TUYA_GPIO_LEVEL_LOW

// LED pin definition
#define BOARD_LED_PIN       TUYA_GPIO_NUM_9
#define BOARD_LED_ACTIVE_LV TUYA_GPIO_LEVEL_HIGH

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
********************function declaration********************
***********************************************************/

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/
OPERATE_RET __board_register_audio(void)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(AUDIO_CODEC_NAME)
    TDD_AUDIO_T5AI_T cfg = {0};
    memset(&cfg, 0, sizeof(TDD_AUDIO_T5AI_T));

    cfg.aec_enable = 1;

    cfg.ai_chn      = TKL_AI_0;
    cfg.sample_rate = TKL_AUDIO_SAMPLE_16K;
    cfg.data_bits   = TKL_AUDIO_DATABITS_16;
    cfg.channel     = TKL_AUDIO_CHANNEL_MONO;

    cfg.spk_sample_rate  = TKL_AUDIO_SAMPLE_16K;
    cfg.spk_pin          = BOARD_SPEAKER_EN_PIN;
    cfg.spk_pin_polarity = TUYA_GPIO_LEVEL_LOW;

    TUYA_CALL_ERR_RETURN(tdd_audio_register(AUDIO_CODEC_NAME, cfg));
#endif
    return rt;
}

static OPERATE_RET __board_register_button(void)
{
    OPERATE_RET       rt = OPRT_OK;
    BUTTON_GPIO_CFG_T button_hw_cfg;

    memset(&button_hw_cfg, 0, sizeof(BUTTON_GPIO_CFG_T));

    // register UP button
    button_hw_cfg.pin                = BOARD_BUTTON_UP_PIN;
    button_hw_cfg.level              = BOARD_BUTTON_UP_ACTIVE_LV;
    button_hw_cfg.mode               = BUTTON_TIMER_SCAN_MODE;
    button_hw_cfg.pin_type.gpio_pull = TUYA_GPIO_PULLUP;

    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(BOARD_BUTTON_NAME_UP, &button_hw_cfg));

    // register DOWN button
    button_hw_cfg.pin                = BOARD_BUTTON_DOWN_PIN;
    button_hw_cfg.level              = BOARD_BUTTON_DOWN_ACTIVE_LV;
    button_hw_cfg.mode               = BUTTON_TIMER_SCAN_MODE;
    button_hw_cfg.pin_type.gpio_pull = TUYA_GPIO_PULLUP;

    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(BOARD_BUTTON_NAME_DOWN, &button_hw_cfg));

    // register ENTER button
    button_hw_cfg.pin                = BOARD_BUTTON_ENTER_PIN;
    button_hw_cfg.level              = BOARD_BUTTON_ENTER_ACTIVE_LV;
    button_hw_cfg.mode               = BUTTON_TIMER_SCAN_MODE;
    button_hw_cfg.pin_type.gpio_pull = TUYA_GPIO_PULLUP;

    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(BOARD_BUTTON_NAME_ENTER, &button_hw_cfg));

    // register RETURN button
    button_hw_cfg.pin                = BOARD_BUTTON_RETURN_PIN;
    button_hw_cfg.level              = BOARD_BUTTON_RETURN_ACTIVE_LV;
    button_hw_cfg.mode               = BUTTON_TIMER_SCAN_MODE;
    button_hw_cfg.pin_type.gpio_pull = TUYA_GPIO_PULLUP;

    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(BOARD_BUTTON_NAME_RETURN, &button_hw_cfg));

    // register LEFT button
    button_hw_cfg.pin                = BOARD_BUTTON_LEFT_PIN;
    button_hw_cfg.level              = BOARD_BUTTON_LEFT_ACTIVE_LV;
    button_hw_cfg.mode               = BUTTON_TIMER_SCAN_MODE;
    button_hw_cfg.pin_type.gpio_pull = TUYA_GPIO_PULLUP;

    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(BOARD_BUTTON_NAME_LEFT, &button_hw_cfg));

    // register RIGHT button
    button_hw_cfg.pin                = BOARD_BUTTON_RIGHT_PIN;
    button_hw_cfg.level              = BOARD_BUTTON_RIGHT_ACTIVE_LV;
    button_hw_cfg.mode               = BUTTON_TIMER_SCAN_MODE;
    button_hw_cfg.pin_type.gpio_pull = TUYA_GPIO_PULLUP;

    TUYA_CALL_ERR_RETURN(tdd_gpio_button_register(BOARD_BUTTON_NAME_RIGHT, &button_hw_cfg));

    return rt;
}

static OPERATE_RET __board_register_led(void)
{
    OPERATE_RET rt = OPRT_OK;

    TDD_LED_GPIO_CFG_T led_gpio;

    led_gpio.pin   = BOARD_LED_PIN;
    led_gpio.level = BOARD_LED_ACTIVE_LV;
    led_gpio.mode  = TUYA_GPIO_PUSH_PULL;

    TUYA_CALL_ERR_RETURN(tdd_led_gpio_register(BOARD_LED_NAME, &led_gpio));

    return rt;
}

static OPERATE_RET __board_register_power_domains(void)
{
    OPERATE_RET rt = OPRT_OK;

    // Initialize power domains (EINK 3V3 and SD card 3V3) and enable by default
    rt = board_power_domain_init();
    if (OPRT_OK != rt) {
        return rt;
    }

    return rt;
}

static OPERATE_RET __board_register_charge_detect(void)
{
    OPERATE_RET rt = OPRT_OK;

    // Initialize charge detect GPIO and interrupt
    // Disabled for now - interrupt mode not supported
    // rt = board_charge_detect_init();
    // if (OPRT_OK != rt) {
    //     return rt;
    // }

    // Initialize battery ADC reading (this works independently of charge detect interrupt)
    // rt = board_battery_adc_init();
    // if (OPRT_OK != rt) {
    //     PR_ERR("Failed to initialize battery ADC: %d", rt);
    //     return rt;
    // }

    return rt;
}

static OPERATE_RET __board_sdio_pin_register(void)
{
    OPERATE_RET rt = OPRT_OK;

    // Configure SDIO pinmux
    tkl_io_pinmux_config(TUYA_GPIO_NUM_14, TUYA_SDIO_HOST_CLK);
    tkl_io_pinmux_config(TUYA_GPIO_NUM_15, TUYA_SDIO_HOST_CMD);
    tkl_io_pinmux_config(TUYA_GPIO_NUM_16, TUYA_SDIO_HOST_D0);
    tkl_io_pinmux_config(TUYA_GPIO_NUM_17, TUYA_SDIO_HOST_D1);
    tkl_io_pinmux_config(TUYA_GPIO_NUM_18, TUYA_SDIO_HOST_D2);
    tkl_io_pinmux_config(TUYA_GPIO_NUM_19, TUYA_SDIO_HOST_D3);

    // Ensure SD card power domain is enabled
    rt = board_power_domain_sd_3v3_enable();
    if (OPRT_OK != rt) {
        return rt;
    }

    // Small delay for power stabilization
    tal_system_sleep(10);

    return OPRT_OK;
}

/**
 * @brief Initialize SD card hardware (power and pinmux)
 *
 * This function ensures the SD card power domain is enabled and SDIO pins are configured.
 * After calling this, applications can mount the SD card using tkl_fs_mount().
 *
 * @return Returns OPERATE_RET_OK on success, or an appropriate error code on failure.
 */
OPERATE_RET board_sdcard_init(void)
{
    OPERATE_RET rt = OPRT_OK;

    // Ensure SD card power domain is enabled
    rt = board_power_domain_sd_3v3_enable();
    if (OPRT_OK != rt) {
        return rt;
    }

    // Small delay for power stabilization
    tal_system_sleep(10);

    return OPRT_OK;
}

/**
 * @brief Registers all the hardware peripherals (audio, button, LED, SD card, power domains) on the board.
 *
 * @return Returns OPERATE_RET_OK on success, or an appropriate error code on failure.
 */
OPERATE_RET board_register_hardware(void)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_LOG(__board_register_power_domains());
    TUYA_CALL_ERR_LOG(__board_register_audio());
    TUYA_CALL_ERR_LOG(__board_register_button());
    TUYA_CALL_ERR_LOG(__board_register_led());
    // Charge detection may fail if interrupt mode not supported - make it non-fatal
    rt = __board_register_charge_detect();
    if (OPRT_OK != rt) {
        PR_WARN("Charge detection initialization failed: %d (continuing without it)", rt);
    }
    TUYA_CALL_ERR_LOG(__board_sdio_pin_register());

    return rt;
}
