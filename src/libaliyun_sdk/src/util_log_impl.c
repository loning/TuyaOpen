/*
 * Copyright 2025 Alibaba Group Holding Ltd.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *     http: *www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "c_utils.h"
#include "tal_log.h"
#include <stdio.h>
#include <stdarg.h>

// Global log level variable - map to TuyaOpen TAL log levels
// uint8_t g_util_log_lv = UTIL_LOG_LV_DEFAULT;

// Map UTIL log levels to TAL log levels
// static TAL_LOG_LEVEL_E util_to_tal_log_level(uint8_t util_level)
// {
//     switch (util_level) {
//     case UTIL_LOG_LV_ERR:
//         return TAL_LOG_LEVEL_ERR;
//     case UTIL_LOG_LV_WARN:
//         return TAL_LOG_LEVEL_WARN;
//     case UTIL_LOG_LV_INFO:
//         return TAL_LOG_LEVEL_INFO;
//     case UTIL_LOG_LV_DEBUG:
//         return TAL_LOG_LEVEL_DEBUG;
//     default:
//         return TAL_LOG_LEVEL_INFO;
//     }
// }

int util_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[512];
    int written = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (written < 0) {
        return written;
    }

    // Trim potential trailing newline for consistent TAL logging
    if (written > 0 && buffer[written - 1] == '\n') {
        buffer[written - 1] = '\0';
    }

    // Attribute SDK logs to the app entry for clearer provenance in output
    tal_log_print(TAL_LOG_LEVEL_INFO, "hello_world.c", 0, "%s", buffer);
    return written;
}

// void util_set_log_level(uint8_t log_lv)
// {
//     g_util_log_lv = log_lv;
//     // Also set the TAL log level
//     tal_log_set_level(util_to_tal_log_level(log_lv));
// }

const char *hex_str(const void *buf, size_t len)
{
    static char hex_buffer[512]; // Static buffer for hex string
    const uint8_t *data = (const uint8_t *)buf;
    size_t i;

    if (len > 255) {
        len = 255; // Limit to prevent buffer overflow
    }

    for (i = 0; i < len; i++) {
        snprintf(hex_buffer + i * 3, 4, "%02X ", data[i]);
    }
    hex_buffer[i * 3 - 1] = '\0'; // Remove trailing space

    return hex_buffer;
}
