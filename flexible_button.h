/**
 * @File:    flexible_button.h
 * @Author:  MurphyZhao
 * @Date:    2018-09-29
 * 
 * Copyright (c) 2018-2018 MurphyZhao <d2014zjt@163.com>
 *               https://github.com/zhaojuntao
 * All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Change logs:
 * Date        Author       Notes
 * 2018-09-29  MurphyZhao   First add
 * 
*/

#ifndef __FLEXIBLE_BUTTON_H__
#define __FLEXIBLE_BUTTON_H__

#include "stdint.h"
#include "string.h"

typedef void (*flex_button_response_callback)(void*);

typedef enum
{
    FLEX_BTN_PRESS_DOWN = 0,
    FLEX_BTN_PRESS_CLICK,
    FLEX_BTN_PRESS_DOUBLE_CLICK,
    FLEX_BTN_PRESS_SHORT_START,
    FLEX_BTN_PRESS_SHORT_UP,
    FLEX_BTN_PRESS_LONG_START,
    FLEX_BTN_PRESS_LONG_UP,
    FLEX_BTN_PRESS_LONG_HOLD,
    FLEX_BTN_PRESS_LONG_HOLD_UP,
    FLEX_BTN_PRESS_MAX,
    FLEX_BTN_PRESS_NONE,
} flex_button_event_t;

/**
 * flex_button_t
 * 
 * @brief Button data structure
 *        Below are members that need to user init before scan.
 * 
 * @member pressed_logic_level:    Logic level when the button is pressed.
 *                                 Must be inited by 'flex_button_register' API
 *                                                     before start button scan.
 * @member debounce_tick:          The time of button debounce.
 *                                 The value is number of button scan cycles.
 * @member click_start_tick:       The time of start click.
 *                                 The value is number of button scan cycles.
 * @member short_press_start_tick: The time of short press start tick.
 *                                 The value is number of button scan cycles.
 * @member long_press_start_tick:  The time of long press start tick.
 *                                 The value is number of button scan cycles.
 * @member long_hold_start_tick:   The time of hold press start tick.
 *                                 The value is number of button scan cycles.
 * @member usr_button_read:        Read the logic level value of specified button.
 * @member cb:                     Button event callback function.
 *                                 If use 'flex_button_event_read' api,
 *                                 you don't need to initialize the 'cb' member.
 * @member next :                  Next button struct
*/
typedef struct flex_button
{
    uint8_t pressed_logic_level : 1; /* need user to init */

    /**
     * @event
     * The event of button in flex_button_evnt_t enum list.
     * Automatically initialized to the default value FLEX_BTN_PRESS_NONE
     *                                      by 'flex_button_register' API.
    */
    uint8_t event               : 4;

    /**
     * @status
     * Used to record the status of the button 
     * Automatically initialized to the default value 0.
    */
    uint8_t status              : 3;
    uint16_t scan_cnt;  /* default 0. Used to record the number of key scans */
    uint16_t click_cnt; /* default 0. Used to record the number of key click */

    uint16_t debounce_tick;          
    uint16_t click_start_tick;
    uint16_t short_press_start_tick;
    uint16_t long_press_start_tick;
    uint16_t long_hold_start_tick;

    uint8_t  (*usr_button_read)(void);
    flex_button_response_callback  cb;
    struct flex_button* next;
} flex_button_t;

#ifdef __cplusplus
extern "C" {
#endif

int8_t flex_button_register(flex_button_t *button);
flex_button_event_t flex_button_event_read(flex_button_t* button);
void flex_button_scan(void);

#ifdef __cplusplus
}
#endif  
#endif /* __FLEXIBLE_BUTTON_H__ */
