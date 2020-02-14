# FlexibleButton

FlexibleButton 是一个基于标准 C 语言的小巧灵活的按键处理库，支持单击、连击、短按、长按、自动消抖，可以自由设置组合按键，可用于中断和低功耗场景。

该按键库解耦了具体的按键硬件结构，理论上支持轻触按键与自锁按键，并可以无限扩展按键数量。另外，FlexibleButton 使用扫描的方式一次性读取所有所有的按键状态，然后通过事件回调机制上报按键事件。核心的按键扫描代码仅有三行，没错，就是经典的 **三行按键扫描算法**。使用 C 语言标准库 API 编写，也使得该按键库可以无缝兼容任意的处理器平台，并且支持任意 OS 和 non-OS（裸机编程）。

## 获取

### Git 方式

```SHELL
git clone https://github.com/murphyzhao/FlexibleButton.git
```

### RT-Thread menuconfig 方式

```
RT-Thread online packages  --->
    miscellaneous packages  --->
        [*] FlexibleButton: Small and flexible button driver  --->
        [*]   Enable flexible button demo
              version (latest)  --->
```

配置完成后，输入 `pkgs --update` 下载软件包。

## 资源统计

ARMCC -O0 优化的情况下，FlexibleButton 资源占用如下：

- CODE：798 字节
- RO DATA：0
- RW DATA：13 字节
- ZI DATA：0

## 快速体验

FlexibleButton 库中提供了一个测试例程 [`./examples/demo_rtt_iotboard.c`](./examples/demo_rtt_iotboard.c)，该例程基于 RT-Thread OS 进行测试，硬件平台选择了 *RT-Thread IoT Board Pandora v2.51* 开发板。当然你可以选择使用其他的 OS，或者使用裸机测试，只需要移除 OS 相关的特性即可。

如果你使用自己的硬件平台，只需要将 FlexibleButton 库源码和例程加入你既有的工程下即可。

## DEMO 程序说明

该示例程序可以直接在 RT-Thread [`stm32l475-atk-pandora`](https://github.com/RT-Thread/rt-thread/tree/master/bsp/stm32/stm32l475-atk-pandora) BSP 中运行，可以在该 BSP 目录下，使用 menuconfig 获取本软件包。

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

如上代码所示，首先使用 `user_button_init();` 初始化用户按键硬件，该步骤将用户按键绑定到 FlexibleButton 库。然后，使用 RT-Thread 的 `INIT_APP_EXPORT` 接口导出为上电自动初始化，创建了一个 “flex_btn” 名字的按键扫描线程，线程里扫描检查按键事件。

### 按键初始化代码

`user_button_init();` 初始化代码如下所示：

```
static void user_button_init(void)
{
    int i;

    /* 初始化按键数据结构 */
    rt_memset(&user_button[0], 0x0, sizeof(user_button));

    /* 初始化 IoT Board 按键引脚，使用 rt-thread PIN 设备框架 */
    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT_PULLUP);    /* 设置 GPIO 为上拉输入模式 */
    rt_pin_mode(PIN_KEY1, PIN_MODE_INPUT_PULLUP);    /* 设置 GPIO 为上拉输入模式 */
    rt_pin_mode(PIN_KEY2, PIN_MODE_INPUT_PULLUP);    /* 设置 GPIO 为上拉输入模式 */
    rt_pin_mode(PIN_WK_UP, PIN_MODE_INPUT_PULLDOWN); /* 设置 GPIO 为下拉输入模式 */

    for (i = 0; i < USER_BUTTON_MAX; i ++)
    {
        user_button[i].id = i;
        user_button[i].usr_button_read = common_btn_read;
        user_button[i].cb = common_btn_evt_cb;
        user_button[i].pressed_logic_level = 0;
        user_button[i].short_press_start_tick = FLEX_MS_TO_SCAN_CNT(1500);
        user_button[i].long_press_start_tick = FLEX_MS_TO_SCAN_CNT(3000);
        user_button[i].long_hold_start_tick = FLEX_MS_TO_SCAN_CNT(4500);

        if (i == USER_BUTTON_3)
        {
            user_button[USER_BUTTON_3].pressed_logic_level = 1;
        }

        flex_button_register(&user_button[i]);
    }
}
```

核心的配置如下：

|配置项|说明|
| :---- | :----|
| id | 按键编号 |
| usr_button_read | 设置按键读值回调函数 |
| cb | 设置按键事件回调函数 |
| pressed_logic_level | 设置按键按下时的逻辑电平 |
| short_press_start_tick | 短按起始 tick，使用 FLEX_MS_TO_SCAN_CNT 宏转化为扫描次数 |
| long_press_start_tick | 长按起始 tick，使用 FLEX_MS_TO_SCAN_CNT 宏转化为扫描次数 |
| long_hold_start_tick | 超长按起始 tick，使用 FLEX_MS_TO_SCAN_CNT 宏转化为扫描次数 |

注意，short_press_start_tick、long_press_start_tick 和 long_hold_start_tick 必须使用 `FLEX_MS_TO_SCAN_CNT` 将毫秒时间转化为扫描次数。

`user_button[i].short_press_start_tick = FLEX_MS_TO_SCAN_CNT(1500);` 表示按键按下开始计时，1500 ms 后按键依旧是按下状态的话，就断定为短按开始。

### 事件处理代码

```C
static void common_btn_evt_cb(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;

    rt_kprintf("id: [%d - %s]  event: [%d - %30s]  repeat: %d\n", 
        btn->id, enum_btn_id_string[btn->id],
        btn->event, enum_event_string[btn->event],
        btn->click_cnt);

    if (flex_button_event_read(&user_button[USER_BUTTON_0]) == flex_button_event_read(&user_button[USER_BUTTON_1]) == FLEX_BTN_PRESS_CLICK)
    {
        rt_kprintf("[combination]: button 0 and button 1\n");
    }
}
```

示例代码中，将所有的按键事件回调均绑定到 `common_btn_evt_cb` 函数，在该函数中打印了按键 ID 和按键事件，以及按键连击次数，并演示了如何使用组合按键。

## FlexibleButton 代码说明

### 按键事件定义

按键事件的定义并没有使用 Windows 驱动上的定义，主要是方便嵌入式设备中的应用场景（也可能是我理解的偏差），按键事件定义如下：

```C
typedef enum
{
    FLEX_BTN_PRESS_DOWN = 0,        // 按下事件
    FLEX_BTN_PRESS_CLICK,           // 单击事件
    FLEX_BTN_PRESS_DOUBLE_CLICK,    // 双击事件
    FLEX_BTN_PRESS_REPEAT_CLICK,    // 连击事件，使用 flex_button_t 中的 click_cnt 断定连击次数
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

其中 `FLEX_BTN_PRESS_LONG_HOLD` 事件可以用来实现长按累加的应用场景。

### 按键数据结构

```
typedef struct flex_button
{
    struct flex_button* next;

    uint8_t  (*usr_button_read)(void *);
    flex_button_response_callback  cb;

    uint16_t scan_cnt;
    uint16_t click_cnt;
    uint16_t max_multiple_clicks_interval;

    uint16_t debounce_tick;
    uint16_t short_press_start_tick;
    uint16_t long_press_start_tick;
    uint16_t long_hold_start_tick;

    uint8_t id;
    uint8_t pressed_logic_level : 1;
    uint8_t event               : 4;
    uint8_t status              : 3;
} flex_button_t;
```

| 序号 | 数据成员 | 是否需要用户初始化 | 说明 |
| :----: | :---- | :----: | :---- |
| 1 | next                   | 否 | 按键库使用单向链表串起所有的按键 |
| 2 | usr_button_read        | 是 | 用户设备的按键引脚电平读取函数，**重要** |
| 3 | cb                     | 是 | 设置按键事件回调，用于应用层对按键事件的分类处理 |
| 4 | scan_cnt               | 否 | 用于记录扫描次数，按键按下是开始从零计数 |
| 5 | click_cnt              | 否 | 记录单击次数，用于判定单击、连击 |
| 6 | max_multiple_clicks_interval  | 是 | 连击间隙，用于判定是否结束连击计数，有默认值 `MAX_MULTIPLE_CLICKS_INTERVAL` |
| 7 | debounce_tick          | 否 | 消抖时间，暂未使用，依靠扫描间隙进行消抖 |
| 8 | short_press_start_tick | 是 | 设置短按事件触发的起始 tick |
| 9 | long_press_start_tick  | 是 | 设置长按事件触发的起始 tick |
| 10 | long_hold_start_tick  | 是 | 设置长按保持事件触发的起始 tick |
| 11 | id                    | 是 | 当多个按键使用同一个回调函数时，用于断定属于哪个按键 |
| 12 | pressed_logic_level   | 是 | 设置按键按下的逻辑电平。1：标识按键按下的时候为高电平；0：标识按键按下的时候未低电平，**重要** |
| 13 | event                 | 否 | 用于记录当前按键事件 |
| 14 | status                | 否 | 用于记录当前按键的状态，用于内部状态机 |

注意，在使用 `max_multiple_clicks_interval`、`debounce_tick`、`short_press_start_tick`、`long_press_start_tick`、`long_hold_start_tick` 的时候，注意需要使用宏 `**FLEX_MS_TO_SCAN_CNT(ms)**` 将毫秒值转换为扫描次数。因为按键库基于扫描次数运转。示例如下：

```
user_button[1].short_press_start_tick = FLEX_MS_TO_SCAN_CNT(1500); // 1500 毫秒
```

上述代码表示：表示按键按下后开始计时，1500ms 的时候，按键依旧按下，则断定为短按开始，并上报 `FLEX_BTN_PRESS_SHORT_START` 事件。

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

按键扫描的核心函数，需要放到应用程序中定时扫描，扫描间隔建议 20 毫秒。

```C
void flex_button_scan(void);
```

## 注意事项

- 阻塞问题

    因为按键事件回调函数以及按键键值读取函数是在按键扫描的过程中执行的，因此请不要在这类函数中使用阻塞接口，不要进行延时操作。

- 按键扫描函数栈需求

    按键扫描函数本身对栈的需求小于 300 字节，但是按键事件回调函数和按键键值读取函数都是在按键扫描函数的上下文中执行的，请格外关心按键事件回调函数与按键键值读取函数对栈空间的需求。

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

该按键库仅做了底层的按键扫描处理，一次扫描可以确定所有的按键状态，并上报对应的按键事件，如果需要支持组合按键，请再封一层，根据按键库返回的事件封装需要的组合按键。[示例程序](./examples/demo_rtt_iotboard.c)提供了简单的实现。

### 关于矩阵键盘

不管你的矩阵键盘是通过什么通信方式获取按键状态的，只要你将读取按键状态的函数对接到 Flexible_button 数据结构中的 `uint8_t  (*usr_button_read)(void*);` 函数上即可。

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
