### 模块：`MotorContrl/Drivers/Board/gpio.c` + `MotorContrl/Inc/gpio.h`

- 扫描时间：2026-09-02 13:12:21 +08:00
- 最终复核时间：2026-09-02 13:20:06 +08:00
- 当前条件编译配置：`USE_HAL_DRIVER`、`STM32F103xE`；未定义调试条件。硬件映射以 `doc/schematic_info.md` 为准。
- 静态扫描：
  - 全局变量、文件静态变量：无。
  - 局部变量：`GPIO_InitStruct`、`step_gpio`、`limit_mask`、`i`，均被读取和写入；函数内 `static const limit_pins[]` 被限位扫描循环读取。
  - 静态函数：`MX_GPIO_Init`，由 `BspGpio_Init` 调用。
  - 公开函数：`BspGpio_Init`、`BspGpio_EnableLimitInterrupts`、`BspGpio_Read`、`BspGpio_Write`、`BspGpio_WriteStepMode`、`BspGpio_WriteXStepMode`、`BspGpio_ReadLimitPin`、`BspGpio_LimitBitFromPin`、`BspGpio_ReadLimitActiveMask`、`BspGpio_LimitMaskForAxis`、`BspGpio_ReadAxisLimitMask`、`BspGpio_ReadXLimitMask`。
  - 宏：头文件保护宏 `__GPIO_H__`，以及 `BSP_GPIO_X_LIMIT_L_BIT`、`BSP_GPIO_X_LIMIT_H_BIT`、`BSP_GPIO_X_LIMIT_R_BIT`、`BSP_GPIO_Y_LIMIT_L_BIT`、`BSP_GPIO_Y_LIMIT_H_BIT`、`BSP_GPIO_Y_LIMIT_R_BIT`、`BSP_GPIO_Z_LIMIT_L_BIT`、`BSP_GPIO_Z_LIMIT_H_BIT`、`BSP_GPIO_Z_LIMIT_R_BIT`。
  - 调试属性命名：无 `DEBUG_`、`dbg_`、`debug_`、`test_`、`tmp_` 符号。
- 引用分析：
  - 9 个限位位宏全部被限位状态映射使用；Y 轴位还被回归用例引用。
  - 初始化、中断使能、通用写、STEP 模式、限位读取/位映射/轴掩码接口均有应用层调用。
  - `BspGpio_Read` 与 `BspGpio_ReadXLimitMask` 当前只有定义和公开声明，没有固件调用。
  - `MX_GPIO_Init` 只有一个直接调用者，但 `tests/check_refactor.py` 会解析该函数体并验证三组双边沿 EXTI 和九路限位引脚，属于冻结结构。
- 删除的变量/宏/函数：无。
- 合并/简化的逻辑：无。曾尝试把 `MX_GPIO_Init` 主体并入 `BspGpio_Init`，但会破坏结构回归检查，已逐字节恢复。
- 保留但标记为“待确认”的项：
  - `BspGpio_Read`、`BspGpio_ReadXLimitMask`：工程内无调用，但可能被工程外 BSP 使用者依赖。
  - `BspGpio_Init -> MX_GPIO_Init` 包装：运行上可合并，但当前测试把 `MX_GPIO_Init` 名称和函数体作为硬件安全契约。
- 修改后代码行数变化：0 行（原 317 行 → 现 317 行）。
- 编译检查：通过。Keil Arm Compiler 6.24 最终全量构建 0 errors、0 warnings。
- 结构检查：通过；候选合并回退后文件与处理前快照 SHA-256 一致。
- 资源结果：`Code=18674`、`RO-data=650`、`RW-data=12`、`ZI-data=2700`。
- 硬件检查：PC14 LED、PC13 LDAC、PC15 RDY、三轴 CS/EN/DIR 及九路限位 EXTI 的引脚、极性、双边沿配置和中断优先级均未改动。
- 待办事项：
  - [ ] 最终确认工程外是否使用两个无内部调用的 BSP 接口。

#### Diff

```diff
# 无最终源码改动；候选初始化合并因结构回归约束已恢复。
```
