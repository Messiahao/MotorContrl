# Firmware releases

This directory holds the exact `.hex` firmware images that passed on-target
verification. The matching source revision is recorded by the Git tag with the
same version name. The current published image is
`v0.13-cn4-serial-link-pass.hex`, which contains the verified CN4/USART3 serial
link test only; it does not add motor motion commands or limit-switch interrupt
handling.
