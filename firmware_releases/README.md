# Firmware releases

This directory holds exact `.hex` firmware images. Images whose tags start with
`v` are reserved for versions that passed the stated on-target verification;
images whose tags start with `dev` are development snapshots and may be
untested. The matching source revision and verification status are recorded by
the Git tag and project version notes. The current published image is
`v0.16-layered-refactor-aux-output-pass.hex`.

The latest archived development snapshot is
`dev-20260901-limit-exti-motion-stop-pass.hex`. It records the user-verified
X-axis continuous-motion limit abort; it is not a formal `v` release because
the ordinary-stop hold-enable change and MCP4728 hardware path are not fully
validated on target.
