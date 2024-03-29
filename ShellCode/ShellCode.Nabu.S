.include "DummyHead.S"

/* Shell Code for Xiaomi Pad 5 Pro */
_ShellCodeStart:
    movz    x4, #0x9004
    movk    x4, #0x390, lsl #16     // Store 0x3909004 to x4, which is io status reg of gpio 9.
    movz    x5, #0x3004
    movk    x5, #0x395, lsl #16     // Store 0x3953004 to x5, which is io status reg of gpio 83.
    ldr     w6, [x4]                // Get status value from address stored in x4 and store it to w6.
    ldr     w7, [x5]                // Get status value from address stored in x5 and store it to w7.
    nop
    nop
    and     w6, w6, w7              // w6 & w7 and store in w6.
    and     w6, w6, #1              // w6 & 1 and stored in w6.
    cbnz     w6, _UEFI               // Compare w6 with 0, if w6 == 0 goto _UEFI, else execute next instruction(jmp Linux).

.include "CommonTail.S"
/* Do not remove the last line  */
