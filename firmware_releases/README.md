# Firmware releases

This directory holds exact `.hex` firmware images. Images whose tags start with
`v` are reserved for versions that passed the stated on-target verification;
images whose tags start with `dev` are development snapshots and may be
untested. The matching source revision and verification status are recorded by
the Git tag and project version notes. The current published image is
`v0.16-layered-refactor-aux-output-pass.hex`.

The latest archived development snapshot is
`dev-20260904-code-cleanup-baseline-unverified.hex`. It records the completed
24-module cleanup, CN6/USART2 host interface, and matching build used as the
next development baseline. It is not a formal `v` release because Y/Z motion,
remaining limits, MCP4728 hardware behavior, and real-time timing are not fully
validated on target.
