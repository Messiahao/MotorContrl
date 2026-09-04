### 模块：`MotorContrl/Inc/stm32f1xx_hal_conf.h`

- 扫描时间：2026-09-04 13:48:03 +08:00
- 当前条件编译配置：Keil AC6.24，定义 `USE_HAL_DRIVER`、`STM32F103xE`；启用 HAL 核心及 GPIO、I2C、SPI、TIM、UART、CORTEX、DMA、FLASH、EXTI、PWR、RCC，未启用 ETH；`USE_FULL_ASSERT` 未定义，所有 HAL 注册回调开关和 `USE_SPI_CRC` 均为 0。
- 静态扫描：
  - 头文件保护宏：`__STM32F1xx_HAL_CONF_H`。
  - HAL 模块宏：`HAL_MODULE_ENABLED` 及 GPIO/I2C/SPI/TIM/UART/CORTEX/DMA/FLASH/EXTI/PWR/RCC 共 12 个模块开关。
  - 振荡器宏：`HSE_VALUE`、`HSE_STARTUP_TIMEOUT`、`HSI_VALUE`、`LSI_VALUE`、`LSE_VALUE`、`LSE_STARTUP_TIMEOUT`。
  - 系统宏：`VDD_VALUE`、`TICK_INT_PRIORITY`、`USE_RTOS`、`PREFETCH_ENABLE`。
  - 回调配置宏：22 个 `USE_HAL_*_REGISTER_CALLBACKS`，均明确配置为 0；SPI 配置宏 `USE_SPI_CRC=0`。
  - 断言宏：`assert_param` 按 `USE_FULL_ASSERT` 条件定义；当前为无代码展开。
  - 变量、函数：无；仅在启用完整断言时声明标准 `assert_failed` 接口。
  - 调试属性：完整断言当前关闭；无 DEBUG/LOG/TRACE 宏或打印缓存。
- 引用分析：
  - GPIO/I2C/SPI/TIM/UART 均由板级驱动直接调用；RCC/FLASH/PWR/CORTEX 参与 HAL 初始化、系统时钟和 NVIC；这些开关不可删除。
  - DMA 与通用 EXTI 驱动当前最终链接段均被移除，但它们属于已加入 Keil 工程的 HAL 公共能力，关闭会裁剪公开 HAL 类型/API，故保守保留。
  - `HAL_ETH_MODULE_ENABLED` 未定义，Keil 工程未包含 ETH 驱动，自研源码也无任何 ETH/PHY 引用；33 个 Ethernet/DP83848 模板宏在当前配置下不可达。
  - `HAL_GPIO_MODULE_ENABLED` 原来定义两次，预处理值完全相同，第二次定义没有作用。
- 删除的变量/宏/函数：
  - 删除重复的第二个 `HAL_GPIO_MODULE_ENABLED` 定义。
  - 删除 33 个未启用 ETH 模块专用宏：6 个 `MAC_ADDR*`、4 个 ETH 缓冲宏、23 个 DP83848/PHY 地址、延时、寄存器及状态宏。
- 合并/简化的逻辑：删除整段不可达的 Ethernet 配置模板，不改任何当前启用模块、时钟、回调或断言行为。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：-57 行（391 行 → 334 行）。
- 编译检查：通过。修改头文件触发 HAL 与全部自研模块重新编译，Keil Arm Compiler 6.24 为 0 errors、0 warnings。
- 结构/行为检查：当前 HAL 模块集合及配置值未变；ETH 在修改前后均未启用；现有回归脚本的 USART 旧基线问题与本模块无关。
- 资源结果：纯预处理模板精简，保持 `Code=17970`、`RO-data=650`、`RW-data=12`、`ZI-data=2444`。
- 待办事项：无。

#### Diff

```diff
 #define HAL_EXTI_MODULE_ENABLED
-#define HAL_GPIO_MODULE_ENABLED  /* 与上方定义重复 */
 #define HAL_PWR_MODULE_ENABLED

-/* Ethernet peripheral configuration：33 个未启用模块专用宏 */
 /* SPI peripheral configuration */
```
