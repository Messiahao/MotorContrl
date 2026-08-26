# MUEYE MotorContrl

STM32F103RC 电机控制固件，当前以 X 轴 TMC5160 驱动器为对象，采用
Keil µVision + Arm Compiler 6 构建。开发过程遵循“先验证通信，再验证寄存器、
使能、单步和短时运动，最后再接入控制协议”的最小验证路线。

## 目录

- `MotorContrl/Src`、`MotorContrl/Inc`：应用代码和 TMC5160 接口。
- `MotorContrl/Drivers`：STM32 HAL/CMSIS 厂商代码。
- `MotorContrl/MDK-ARM`：Keil 工程文件。
- `doc/schematic_info.md`：硬件引脚和连接关系的唯一参考。
- `doc/risk_and_development_plan.md`：风险、开发路线和实机验证记录。
- `firmware_releases`：已通过对应实机验证的 HEX 文件。

## 版本路线

下表中的“本版功能”是该标签对应代码的实际范围；每个标签都保留了完整源代码，
可以用 Git 切换回当时的状态。

| 标签 | 相对前一归档版本的主要改动 | 本版功能与验证状态 |
|---|---|---|
| `v0.3-gstat-clear-pass` | 建立 TMC5160 启动后的状态读取和 GSTAT 清除步骤。 | 读取 SPI 身份，读取并清除 GSTAT 锁存位；等待后确认 UV_CP 未再次出现。 |
| `v0.4-chopconf-read-pass` | 在 GSTAT 验证后增加 CHOPCONF 静态读回。 | 读取并记录复位值 `0x10410150`，不使能驱动、不发 STEP。 |
| `v0.5-low-current-config-pass` | 增加最小电流和保守斩波配置写入。 | `IHOLD_IRUN=0`，保留 CHOPCONF 其他位并将 TOFF 设为 3；读回 `0x10410153`。 |
| `v0.6-enable-no-step-pass` | 在配置通过后增加驱动使能和状态检查。 | 低电流使能 X 轴，读取 GSTAT/DRV_STATUS；无故障保持约 5 秒后自动禁用，不发 STEP。 |
| `v0.7-single-step-pass` | 增加一次受控 STEP，并临时将 PA1 从 TIM2 复用输出切换为 GPIO。 | 发送 1 个 1 ms 高、1 ms 低脉冲；MSCNT 变化 `+1`，随后恢复 PA1 复用配置。 |
| `v0.8-sixteen-step-pass` | 在单步通过后增加有限脉冲批次。 | 固定方向，发送 16 个约 20 Hz 脉冲；MSCNT 变化 `+16`，确认输入通路连续可靠。 |
| `v0.9-low-current-motion-pass` | 将计数测试扩展为定时短时运动。 | 最小电流下约 500 Hz 运行 4 秒，理论约 14°；手感确认电机连续运动，结束后自动禁用。 |
| `v0.10-limit-static-input-deferred` | 暂停电机运动验证，加入限位输入静态读取框架。 | 配置 9 路限位输入，暂不启用 EXTI；因现有限位光电开关损坏，静态输入尚未宣称实机通过，因此没有对应 HEX。 |

## 当前硬件与安全状态

- X 轴 TMC5160：SPI1，CS 为 PC2，ENN 为 PC3，DIR 为 PA0，STEP 为 PA1。
- X 电机接在 CN17；同一绕组接在同一相输入，绕组极性可通过 DIR/后续逻辑调整。
- 当前验证使用最小电流档，仅适合空载、短时测试；不要直接用于长期连续运行或大负载。
- PB12/PB13 的 5 V、24 V 输出试验代码已注释。此前 Q4 的 D-S 测得短路，待硬件更换确认后再恢复相关开发。
- 限位开关到货并确认接线前，不进行回零或限位停机测试。

## 查看和构建

切换到某个历史版本，例如：

```text
git switch --detach v0.8-sixteen-step-pass
```

在 Keil 中打开 `MotorContrl/MDK-ARM/MotorContrl.uvprojx`，执行 **Build Target**。
对应的已验证 HEX 位于 `firmware_releases`。返回最新开发分支：

```text
git switch master
```

后续开发顺序记录在 `doc/risk_and_development_plan.md`：先完成限位 GPIO 的实机验证，
再冻结 USART2 串口帧格式，之后才允许通过串口控制单轴低速短时运动。
