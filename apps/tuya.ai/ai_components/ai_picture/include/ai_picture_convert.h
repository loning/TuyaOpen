/**
 * @file ai_picture_convert.h
 * @version 0.1
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */

#ifndef __AI_PICTURE_CONVERT_H__
#define __AI_PICTURE_CONVERT_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
typedef struct {
    TUYA_FRAME_FMT_E in_fmt;
    uint32_t         in_frame_size;
    TUYA_FRAME_FMT_E out_fmt;
} AI_PICTURE_CONVERT_CFG_T;

typedef struct {
    TUYA_FRAME_FMT_E fmt;
    uint16_t         width;
    uint16_t         height;
    uint8_t         *frame;
    uint32_t         frame_size;
} AI_PICTURE_INFO_T;

/***********************************************************
********************function declaration********************
***********************************************************/
OPERATE_RET ai_picture_convert_start(AI_PICTURE_CONVERT_CFG_T *cfg);

OPERATE_RET ai_picture_convert_feed(uint8_t *data, uint32_t len);

OPERATE_RET ai_picture_convert(AI_PICTURE_INFO_T *out_info);

OPERATE_RET ai_picture_convert_stop(void);


#ifdef __cplusplus
}
#endif

#endif /* __AI_PICTURE_CONVERT_H__ */
