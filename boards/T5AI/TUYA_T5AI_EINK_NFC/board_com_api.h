/**
 * @file board_com_api.h
 * @author Tuya Inc.
 * @brief Header file for common board-level hardware registration APIs.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 */

#ifndef __BOARD_COM_API_H__
#define __BOARD_COM_API_H__

#include "tuya_cloud_types.h"
#include "board_power_domain_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/
// Button name definitions
#define BOARD_BUTTON_NAME_UP     "btn_up"
#define BOARD_BUTTON_NAME_DOWN   "btn_down"
#define BOARD_BUTTON_NAME_ENTER  "btn_enter"
#define BOARD_BUTTON_NAME_RETURN "btn_return"
#define BOARD_BUTTON_NAME_LEFT   "btn_left"
#define BOARD_BUTTON_NAME_RIGHT  "btn_right"

#define BOARD_LED_NAME "user_led"

/***********************************************************
********************function declaration********************
***********************************************************/
// Forward declarations for battery functions
// Full declarations are in board_charge_detect_api.h
OPERATE_RET board_battery_read(uint32_t *voltage_mv, uint8_t *percentage);

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
********************function declaration********************
***********************************************************/

/**
 * @brief Initialize SD card hardware (power and pinmux)
 *
 * This function ensures the SD card power domain is enabled and SDIO pins are configured.
 * After calling this, applications can mount the SD card using tkl_fs_mount().
 *
 * @return Returns OPERATE_RET_OK on success, or an appropriate error code on failure.
 */
OPERATE_RET board_sdcard_init(void);

/**
 * @brief Registers all the hardware peripherals (audio, button, SD card) on the board.
 *
 * @return Returns OPERATE_RET_OK on success, or an appropriate error code on failure.
 */
OPERATE_RET board_register_hardware(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_COM_API_H__ */
