# FlexibleButton

FlexibleButton 是一个基于 C 语言的小巧灵活的按键处理库。该按键库解耦了具体的按键硬件结构，使用扫描的方式一次性读取所有所有的按键状态，然后通过事件回调机制上报按键事件。FlexibleButton 库预置最大支持八个按键处理，但理论上可以无限扩展按键数量。

## 使用方法

请参考 [`./flexible_button_demo.c`](./flexible_button_demo.c)。
