# Firmware releases

This directory holds the exact `.hex` firmware images that passed on-target
verification. The matching source revision is recorded by the Git tag with the
same version name. The current published image is
`v0.14-10khz-1a-motion-pass.hex`, which contains the verified short X-axis
motion observation in addition to the CN4/USART3 link test; it does not add
serial motion commands or limit-switch interrupt handling.
