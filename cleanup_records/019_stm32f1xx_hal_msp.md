### 模块：`MotorContrl/Src/stm32f1xx_hal_msp.c`

- 扫描时间：2026-09-04 13:39:42 +08:00
- 当前条件编译配置：Keil AC6.24，定义 `USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。模块内无条件编译分支。
- 静态扫描：
  - 全局/静态变量：无。
  - 宏定义、私有类型、私有函数：无。
  - 公开回调：`HAL_MspInit`。
  - include：`main.h`，为 HAL RCC/AFIO 宏提供统一工程入口。
  - 调试属性命名：无调试变量、打印缓冲、测试函数或计数器。
- 引用分析：
  - `HAL_MspInit` 由 HAL 库的 `HAL_Init` 强制调用，并覆盖库内 `__weak` 默认实现，不能删除或改为静态函数。
  - `__HAL_RCC_AFIO_CLK_ENABLE` 为 AFIO 重映射提供时钟；`__HAL_RCC_PWR_CLK_ENABLE` 保持 CubeMX 全局 MSP 初始化。
  - `__HAL_AFIO_REMAP_SWJ_NOJTAG` 禁用 JTAG-DP、保留 SW-DP，与原理图 PA13/SWDIO、PA14/SWCLK 调试接口一致；不能删除。
  - 文件中的空 `USER CODE` 区域和段落标记属于 CubeMX 再生成边界，不产生目标码。
- 删除的变量/宏/函数：无。
- 合并/简化的逻辑：无。该文件属于 CubeMX 生成边界，删除空注释不会减少 Flash/RAM，反而增加后续再生成冲突。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：0 行（85 → 85）。
- 编译检查：通过。处理前已使用 Keil Arm Compiler 6.24 完整重建当前源码，0 errors、0 warnings；本模块未修改。
- 结构/行为检查：`python tests/check_refactor.py --structure-only` 通过，确认 MSP 文件与冻结基线一致。
- 资源结果：无源码修改，保持 `Code=18170`、`RO-data=650`、`RW-data=12`、`ZI-data=2516`。
- 待办事项：无。

#### Diff

```diff
# 经全工程引用、硬件映射和冻结结构检查后无需修改。
```
