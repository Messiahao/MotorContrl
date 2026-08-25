# Repository Guidelines

## Project Structure & Module Organization

This repository contains STM32F103RC motor-control firmware. Work primarily
under `MotorContrl/`:

- `Src/` holds application and CubeMX-generated C sources; `main.c` is the
  entry point and `tmc5160.c` implements the motor-driver interface.
- `Inc/` contains matching public headers. Keep each module's `.c` and `.h`
  filenames aligned (for example, `tmc5160.c` and `tmc5160.h`).
- `App/` is reserved for application-level code.
- `Drivers/` contains vendor HAL and CMSIS code; do not edit it unless an
  upstream/vendor change is intended.
- `MDK-ARM/MotorContrl.uvprojx` is the Keil µVision project; generated build
  artifacts are stored below `MDK-ARM/MotorContrl/`.
- `MotorContrl.ioc` is the STM32CubeMX configuration. Regenerate only when its
  configuration changes, then review generated-code diffs carefully.

Hardware reference material lives in `doc/`.

## Build, Test, and Development Commands

Open `MotorContrl/MDK-ARM/MotorContrl.uvprojx` in Keil µVision (Arm Compiler
6, STM32F1 DFP 2.4.1) and use **Project > Build Target** to produce the ELF and
HEX image. From a Developer Command Prompt with µVision on `PATH`, the
equivalent batch build is:

```powershell
UV4.exe -b MotorContrl\MDK-ARM\MotorContrl.uvprojx
```

No repository-owned automated test target is configured. Treat a clean build
and on-target verification of the affected peripheral or motor behavior as the
required check.

## Coding Style & Naming Conventions

Use C and follow the surrounding module's formatting rather than reformatting
unrelated generated files. Preserve HAL/CubeMX naming such as `MX_SPI1_Init`,
`HAL_SPI_Transmit`, and `GPIO_PIN_0`. Use `snake_case` for new source/header
filenames, `UPPER_SNAKE_CASE` for macros, and descriptive `PascalCase` names
for project-level functions and types (for example, `TMC5160_ReadRegister`).
Keep hardware register values, timing assumptions, and pin behavior documented
next to the code that uses them.

## Changes, Commits, and Reviews

There is no Git history in this checkout, so no established commit-message
convention can be inferred. Use concise imperative subjects, for example:
`Add TMC5160 stallguard configuration`.

Keep each change focused. In a review or handoff, state the target board,
peripherals touched, CubeMX/project-file changes, build result, and on-target
test performed. Include serial logs or screenshots for observable behavior
when useful, and never commit generated object, listing, or IDE user files
unless the change requires them.


## Hardware Schematic Memory (Project Knowledge Base)

- The hardware schematic PDF is located in `doc/`. Do not re-parse the PDF from scratch in every new conversation session.

- **First-time setup**: When the schematic is first mentioned or when a major hardware revision occurs, read the PDF, extract critical hardware mappings (e.g., MCU pin assignments, SPI connections to TMC5160, power rails, encoder/limit switch GPIOs), and generate a permanent summary file at `doc/schematic_info.md`.

- **Reference rule**: For all subsequent coding tasks (e.g., writing initialization code, configuring SPI, or setting up GPIO interrupts), ALWAYS refer to `doc/schematic_info.md` as the single source of truth for pinouts and hardware connections. Only fall back to re-reading the PDF if the user explicitly states that the schematic has been updated.

- **Update protocol**: If you suspect a mismatch between the code's current pin usage and `doc/schematic_info.md`, stop and ask the user to confirm the correct pin before proceeding. Once confirmed, update both the code and the `schematic_info.md` file accordingly.
