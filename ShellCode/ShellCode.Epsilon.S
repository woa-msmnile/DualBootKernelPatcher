.include "DummyHead.S"

/* Shell Code for Surface Duo 1 */
_ShellCodeStart:
    movz    x4, #0x9004
    movk    x4, #0x3d7, lsl #16     // Store 0x3D79004 to x4, which is io status reg of gpio 121.
    ldr     w5, [x4]                // Get status value from address stored in x4 and store it to w5.
    and     w5, w5, #1              // w5 & 1 and save value in w5.
    cbz     w5, _UEFI               // Compare w5 with 0, if w5 == 0 goto _UEFI, else execute next instruction(jmp Linux).

.include "CommonTail.S"
/* Do not remove the last line  */
