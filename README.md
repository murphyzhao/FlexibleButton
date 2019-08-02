# FlexibleButton

FlexibleButton 是一个基于 C 语言的小巧灵活的按键处理库。该按键库解耦了具体的按键硬件结构，理论上支持轻触按键与自锁按键，并可以无限扩展按键数量。另外，FlexibleButton 使用扫描的方式一次性读取所有所有的按键状态，然后通过事件回调机制上报按键事件。

该按键库使用 C 语言编写，驱动与应用程序解耦，便于灵活应用，比如用户可以方便地在应用层增加按键中断、处理按键功耗、定义按键事件处理方式，而无需修改 FlexibleButton 库中的代码。核心的按键扫描代码仅有三行，没错，就是经典的 **三行按键扫描算法**。使用 C 语言标准库 API 编写，也使得该按键库可以无缝兼容任意的处理器平台，并且支持任意 OS 和 non-OS（裸机程序）。

## 获取方式

```SHELL
git clone https://github.com/murphyzhao/FlexibleButton.git
```

## 使用与测试

FlexibleButton 库中提供了一个测试例程 [`./flexible_button_demo.c`](./flexible_button_demo.c)，该例程基于 RT-Thread OS 进行测试，硬件平台选择了 RT-Thread IoT Board v2.2 开发板。当然你可以选择使用其他的 OS，或者使用裸机测试，只需要移除 OS 相关的特性即可。

如果你使用自己的硬件平台，只需要将 FlexibleButton 库源码和例程加入你既有的工程下即可。

## DEMO 程序说明

### 确定用户按键

```C
typedef enum
{
    USER_BUTTON_0 = 0, // 对应 IoT Board 开发板的 PIN_KEY0
    USER_BUTTON_1,     // 对应 IoT Board 开发板的 PIN_KEY1
    USER_BUTTON_2,     // 对应 IoT Board 开发板的 PIN_KEY2
    USER_BUTTON_3,     // 对应 IoT Board 开发板的 PIN_WK_UP
    USER_BUTTON_MAX
} user_button_t;

static flex_button_t user_button[USER_BUTTON_MAX];
```

上述代码定义了 4 个按键，数据结构存储在 `user_button` 数组中。

### 程序入口

```C
int flex_button_main(void)
{
    rt_thread_t tid = RT_NULL;
    user_button_init();
    /* 创建按键扫描线程 flex_btn，线程栈 1024 byte，优先级 10 */
    tid = rt_thread_create("flex_btn", button_scan, RT_NULL, 1024, 10, 10);
    if(tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    return 0;
}
/* 使用 RT-Thread 的自动初始化 */
INIT_APP_EXPORT(flex_button_main);
```

如上所示，首先使用 `user_button_init();` 初始化用户按键硬件，该步骤将用户按键绑定到 FlexibleButton 库。然后，使用 RT-Thread 的 `INIT_APP_EXPORT` 接口导出为上电自动初始化，创建了一个 “flex_btn” 名字的按键扫描线程，线程里扫描检查按键事件。

### 按键初始化代码

`user_button_init();` 初始化代码如下所示：

```C
static void user_button_init(void)
{
    int i;

    /* 初始化按键数据结构 */
    rt_memset(&user_button[0], 0x0, sizeof(user_button));
    user_button[USER_BUTTON_0].usr_button_read = button_key0_read;
    user_button[USER_BUTTON_0].cb = (flex_button_response_callback)btn_0_cb;
    user_button[USER_BUTTON_1].usr_button_read = button_key1_read;
    user_button[USER_BUTTON_1].cb = (flex_button_response_callback)btn_1_cb;
    user_button[USER_BUTTON_2].usr_button_read = button_key2_read;
    user_button[USER_BUTTON_3].usr_button_read = button_keywkup_read;

    /* 初始化 IoT Board 按键引脚，使用 rt-thread PIN 设备框架 */
    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT); /* 设置 GPIO 为输入模式 */
    rt_pin_mode(PIN_KEY1, PIN_MODE_INPUT); /* 设置 GPIO 为输入模式 */
    rt_pin_mode(PIN_KEY2, PIN_MODE_INPUT); /* 设置 GPIO 为输入模式 */
    rt_pin_mode(PIN_WK_UP, PIN_MODE_INPUT);/* 设置 GPIO 为输入模式 */

    /* 核心的按键配置
     * pressed_logic_level，设置按键按下的逻辑电平
     * click_start_tick，设置触发按键按下事件的起始 tick（用于消抖）
     * short_press_start_tick，设置短按事件触发的起始 tick
     * long_press_start_tick，设置长按事件触发的起始 tick
     * long_hold_start_tick，设置长按保持事件触发的起始 tick
     */
    for (i = 0; i < USER_BUTTON_MAX; i ++)
    {
        user_button[i].pressed_logic_level = 0;
        user_button[i].click_start_tick = 20;
        user_button[i].short_press_start_tick = 100;
        user_button[i].long_press_start_tick = 200;
        user_button[i].long_hold_start_tick = 300;
        if (i == USER_BUTTON_3)
        {
            user_button[USER_BUTTON_3].pressed_logic_level = 1;
        }
        flex_button_register(&user_button[i]);
    }
}
```

 `user_button_init();` 主要用于初始化按键硬件，设置按键长按和短按的 tick 数（RT-Thread RT_TICK_PER_SECOND 配置为 1000 时，默认一个 tick 为 1ms），然后注册按键到 FlexibleButton 库。

### 事件处理代码

```C
static void btn_0_cb(flex_button_t *btn)
{
    rt_kprintf("btn_0_cb\n");
    switch (btn->event)
    {
        case FLEX_BTN_PRESS_DOWN:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_DOWN]\n");
            break;
        case FLEX_BTN_PRESS_CLICK:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_CLICK]\n");
            break;
        case FLEX_BTN_PRESS_DOUBLE_CLICK:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_DOUBLE_CLICK]\n");
            break;
        case FLEX_BTN_PRESS_SHORT_START:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_SHORT_START]\n");
            break;
        case FLEX_BTN_PRESS_SHORT_UP:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_SHORT_UP]\n");
            break;
        case FLEX_BTN_PRESS_LONG_START:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_LONG_START]\n");
            break;
        case FLEX_BTN_PRESS_LONG_UP:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_LONG_UP]\n");
            break;
        case FLEX_BTN_PRESS_LONG_HOLD:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_LONG_HOLD]\n");
            break;
        case FLEX_BTN_PRESS_LONG_HOLD_UP:
            rt_kprintf("btn_0_cb [FLEX_BTN_PRESS_LONG_HOLD_UP]\n");
            break;
    }
}
```

## FlexibleButton 代码说明

### 按键事件定义

按键事件的定义并没有使用 Windows 驱动上的定义，主要是方便嵌入式设备中的应用场景（也可能是我理解的偏差），按键事件定义如下：

```C
typedef enum
{
    FLEX_BTN_PRESS_DOWN = 0,        // 按下事件
    FLEX_BTN_PRESS_CLICK,           // 单击事件
    FLEX_BTN_PRESS_DOUBLE_CLICK,    // 双击事件
    FLEX_BTN_PRESS_SHORT_START,     // 短按开始事件
    FLEX_BTN_PRESS_SHORT_UP,        // 短按抬起事件
    FLEX_BTN_PRESS_LONG_START,      // 长按开始事件
    FLEX_BTN_PRESS_LONG_UP,         // 长按抬起事件
    FLEX_BTN_PRESS_LONG_HOLD,       // 长按保持事件
    FLEX_BTN_PRESS_LONG_HOLD_UP,    // 长按保持的抬起事件
    FLEX_BTN_PRESS_MAX,
    FLEX_BTN_PRESS_NONE,
} flex_button_event_t;
```

### 按键数据结构

```
typedef struct flex_button
{
    uint8_t pressed_logic_level : 1;
    uint8_t event               : 4;
    uint8_t status              : 3;
    uint16_t scan_cnt;
    uint16_t click_cnt;
    uint16_t debounce_tick;          
    uint16_t click_start_tick;
    uint16_t short_press_start_tick;
    uint16_t long_press_start_tick;
    uint16_t long_hold_start_tick;
    uint8_t  (*usr_button_read)(void);
    flex_button_response_callback  cb;
    struct flex_button* next;
} flex_button_t;
```

| 序号 | 数据成员 | 是否需要用户初始化 | 说明 |
| :----: | :---- | :----: | :---- |
| 1 | pressed_logic_level | 是 | 设置按键按下的逻辑电平。1：高电平为按下状态；0：低电平为按下状态 |
| 2 | event               | 否 | 用于记录当前按键的按键事件 |
| 3 | status              | 否 | 用于记录当前按键的状态，按下、抬起、长按等 |
| 4 | scan_cnt            | 否 | 用于记录扫描次数 |
| 5 | click_cnt           | 否 | 用于记录单击次数，用于判断双击事件 |
| 6 | debounce_tick       | 是 | 消抖时间，默认为 0，可以不配置，依靠扫描间隙进行消抖 |
| 7 | click_start_tick    | 是 | 设置触发按键按下事件的起始 tick，有消抖效果 |
| 8 | short_press_start_tick | 是 | 设置短按事件触发的起始 tick |
| 9 | long_press_start_tick  | 是 | 设置长按事件触发的起始 tick |
| 10 | long_hold_start_tick   | 是 | 设置长按保持事件触发的起始 tick |
| 11 | usr_button_read        | 是 | 用户设备的按键引脚电平读取函数，**重要** |
| 12 | cb                     | 是 | 设置按键事件回调，用于应用层对按键事件的分类处理 |
| 13 | next                   | 否 | 按键库使用单向链表串起所有的按键配置 |

### 按键注册接口

使用该接口注册一个用户按键，入参为一个 flex_button_t 结构体实例的地址。

```C
int8_t flex_button_register(flex_button_t *button);
```

### 按键事件读取接口

使用该接口获取指定按键的事件。

```C
flex_button_event_t flex_button_event_read(flex_button_t* button);
````

### 按键扫描接口

按键扫描的核心函数，需要放到应用程序中定时扫描间隔 5-20ms 即可。

```C
void flex_button_scan(void);
```

## 其它

### 关于低功耗

本按键库是通过不间断扫描的方式来检查按键状态，因此会一直占用 CPU 资源，这对低功耗应用场景是不友好的。为了降低正常工作模式下的功耗，建议合理配置扫描周期（5ms - 20ms），扫描间隙里 CPU 可以进入轻度睡眠。

该按键库不在底层实现低功耗处理，应用层可以根据自己的功耗模式灵活处理，通常会有以下两种方式：

1. 进入低功耗前，挂起按键扫描线程；退出低功耗后，唤醒按键扫描。
2. 增加按键中断模式，所有的按键中断来，就触发一次按键扫描，以确认所有的按键状态。

> 低功耗相关的探讨参考 [issue 1](https://github.com/murphyzhao/FlexibleButton/issues/1) 中的讨论。

### 关于按键中断模式

由于该按键库一次扫描可以确定所有的按键状态，因此可以将所有的按键中断通过 “**或**” 的方式转化为一个中断，然后在中断处理函数中执行一次按键扫描。

中断 “**或**” 的方式可以通过硬件来完成，也可以通过软件来完成。

硬件方式，需要使用一个 **或门** 芯片，多个输入条件转化为一个输出条件，然后通过一个外部中断即可完成所有按键的中断方式检测。

软件方式，需要为每一个按键配置为中断触发模式，然后在每一个按键中断的中断处理函数中执行按键扫描。

为了在降低中断处理函数中执行按键扫描带来的时延，可以通过信号量的方式来异步处理，仅在中断处理函数中释放一个按键扫描的信号量，然后在按键扫描线程中监测该信号量。

### 关于组合按键

该按键库仅做了底层的按键扫描处理，一次扫描可以确定所有的按键状态，并上报对应的按键事件，如果需要支持组合按键，请再封一层，根据按键库返回的事件封装需要的组合按键。

### 关于矩阵键盘

不管你的矩阵键盘是通过什么通信方式获取按键状态的，只要你将读取按键状态的函数对接到 Flexible_button 数据结构中的 `uint8_t  (*usr_button_read)(void);` 函数上即可。

> 参考 [issue 2](https://github.com/murphyzhao/FlexibleButton/issues/2) 中的讨论。

## 问题和建议

如果有什么问题或者建议欢迎提交 [Issue](https://github.com/murphyzhao/FlexibleButton/issues) 进行讨论。

## 维护

- [MurphyZhao](https://github.com/murphyzhao)

## 感谢

感谢所有一起探讨的朋友，感谢所有使用 flexible_button 的朋友，感谢你们的 Star 和 Fork，谢谢你们的支持。

- 感谢 [BOBBOM](https://github.com/BOBBOM) 发现 flex_button_register 函数中的逻辑问题
- 感谢 [BOBBOM](https://github.com/BOBBOM) 解除 flexible_button 中对按键数量的限制
- 感谢 [**rt-thread**](https://mp.weixin.qq.com/s/HJEcSXhykBq1T5Hx0TdjMw) 的支持
- 感谢 [**电子发烧友**](https://mp.weixin.qq.com/s/mQFyrPAvz_TSktQLrSqQfA) 的支持
- 感谢 [**威驰电子**](https://mp.weixin.qq.com/s/oAwFXPostMFBtb2EGxTdig) 的支持

## 友情链接

- RT-Thread [IoT Board](https://github.com/RT-Thread/IoT_Board) 开发板
