/**
 * @File:    demo_tos_evb_mx_plus.c
 * @Author:  MurphyZhao
 * @Date:    2018-09-29
 * 
 * Copyright (c) 2018-2019 MurphyZhao <d2014zjt@163.com>
 *               https://github.com/murphyzhao
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
 * message:
 * This demo is base on TencentOSTiny EVB_MX+, reference
 *     https://github.com/Tencent/TencentOS-tiny
 * Hardware: TencentOSTiny EVB_MX+.
 * 
 * Change logs:
 * Date        Author       Notes
 * 2020-01-20  MurphyZhao   First add
 * 2020-02-14  MurphyZhao   Fix Key mismatch problem and fix grammar bug
*/

#include "stdio.h"
#include "mcu_init.h"
#include "cmsis_os.h"

#include "main.h"

#include "flexible_button.h"

#define FLEXIBLE_BTN_STK_SIZE          512
static void button_scan(void *arg);
osThreadDef(button_scan, osPriorityNormal, 1, FLEXIBLE_BTN_STK_SIZE);

#ifndef PIN_KEY4
#define PORT_KEY4 GPIOB
#define PIN_KEY4 GPIO_PIN_12 // PB12
#endif

#ifndef PIN_KEY3
#define PORT_KEY3 GPIOB
#define PIN_KEY3 GPIO_PIN_2  // PB2
#endif

#ifndef PIN_KEY2
#define PORT_KEY2 GPIOC
#define PIN_KEY2 GPIO_PIN_10 // PC10
#endif

#ifndef PIN_KEY1
#define PORT_KEY1 GPIOB
#define PIN_KEY1 GPIO_PIN_13 // PB13
#endif

#define ENUM_TO_STR(e) (#e)

typedef enum
{
    USER_BUTTON_1 = 0,
    USER_BUTTON_2,
    USER_BUTTON_3,
    USER_BUTTON_4,
    USER_BUTTON_MAX
} user_button_t;

static char *enum_event_string[] = {
    ENUM_TO_STR(FLEX_BTN_PRESS_DOWN),
    ENUM_TO_STR(FLEX_BTN_PRESS_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_DOUBLE_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_REPEAT_CLICK),
    ENUM_TO_STR(FLEX_BTN_PRESS_SHORT_START),
    ENUM_TO_STR(FLEX_BTN_PRESS_SHORT_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_START),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_HOLD),
    ENUM_TO_STR(FLEX_BTN_PRESS_LONG_HOLD_UP),
    ENUM_TO_STR(FLEX_BTN_PRESS_MAX),
    ENUM_TO_STR(FLEX_BTN_PRESS_NONE),
};

static char *enum_btn_id_string[] = {
    ENUM_TO_STR(F1),
    ENUM_TO_STR(F2),
    ENUM_TO_STR(F3),
    ENUM_TO_STR(F4),
    ENUM_TO_STR(USER_BUTTON_MAX),
};

static flex_button_t user_button[USER_BUTTON_MAX];

static uint8_t common_btn_read(void *arg)
{
    uint8_t value = 0;

    flex_button_t *btn = (flex_button_t *)arg;

    switch (btn->id)
    {
    case USER_BUTTON_1:
        value = HAL_GPIO_ReadPin(PORT_KEY1, PIN_KEY1);
        break;
    case USER_BUTTON_2:
        value = HAL_GPIO_ReadPin(PORT_KEY2, PIN_KEY2);
        break;
    case USER_BUTTON_3:
        value = HAL_GPIO_ReadPin(PORT_KEY3, PIN_KEY3);
        break;
    case USER_BUTTON_4:
        value = HAL_GPIO_ReadPin(PORT_KEY4, PIN_KEY4);
        break;
    default:
        break;
    }

    return value;
}

static void common_btn_evt_cb(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;

    printf("id: [%d - %s]  event: [%d - %30s]  repeat: %d\r\n", 
        btn->id, enum_btn_id_string[btn->id],
        btn->event, enum_event_string[btn->event],
        btn->click_cnt);

    if ((flex_button_event_read(&user_button[USER_BUTTON_1]) == FLEX_BTN_PRESS_CLICK) &&\
        (flex_button_event_read(&user_button[USER_BUTTON_2]) == FLEX_BTN_PRESS_CLICK))
    {
        printf("[combination]: button 1 and button 2\r\n");
    }
}

static void button_scan(void *arg)
{
    while(1)
    {
        flex_button_scan();
        osDelay(20); // 20 ms
    }
}

static void user_button_init(void)
{
    int i;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    memset(&user_button[0], 0x0, sizeof(user_button));
    
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = PIN_KEY1 | PIN_KEY3 | PIN_KEY4;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PIN_KEY2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    for (i = 0; i < USER_BUTTON_MAX; i ++)
    {
        user_button[i].id = i;
        user_button[i].usr_button_read = common_btn_read;
        user_button[i].cb = common_btn_evt_cb;
        user_button[i].pressed_logic_level = 0;
        user_button[i].short_press_start_tick = FLEX_MS_TO_SCAN_CNT(1500);
        user_button[i].long_press_start_tick = FLEX_MS_TO_SCAN_CNT(3000);
        user_button[i].long_hold_start_tick = FLEX_MS_TO_SCAN_CNT(4500);

        flex_button_register(&user_button[i]);
    }
}

/**
 * flex_button_main
 * 
 * @brief please call this function in application.
 * 
*/
int flex_button_main(void)
{
    user_button_init();

    /* Create background ticks thread */
    osThreadCreate(osThread(button_scan), NULL);

    return 0;
}
