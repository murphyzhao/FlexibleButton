# FlexibleButton

FlexibleButton 是一个基于 C 语言的小巧灵活的按键处理库。该按键库解耦了具体的按键硬件结构，理论上支持轻触按键与自锁按键，并可以无限扩展按键数量。另外，FlexibleButton 使用扫描的方式一次性读取所有所有的按键状态，然后通过事件回调机制上报按键事件。

## 获取方式

```SHELL
git clone https://github.com/zhaojuntao/FlexibleButton.git
```

## 使用方法

请参考 [`./flexible_button_demo.c`](./flexible_button_demo.c)。该例程基于 RT-Thread IoT Board v2.2 开发板测试.

## TODO

- 支持自锁按键
- 优化低功耗使用场景
- 增加使用说明

## 问题和建议

如果有什么问题或者建议欢迎在这个 [Issue](https://github.com/zhaojuntao/FlexibleButton/issues/1) 上讨论。

## 贡献者

- [MurphyZhao](https://github.com/zhaojuntao)
