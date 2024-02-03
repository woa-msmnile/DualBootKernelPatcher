.include "DummyHead.S"

_ShellCodeStart:
    movz    x4, #0xd000
    movk    x4, #0x317, lsl #16     // Store 0x317d004 to x4, which is control reg of gpio 125.
    str     wzr, [x4], #4           // Set 0 in control reg, then x4 add 4
    ldr     w5, [x4]                // Get status value from address stored in x4 and store it to x5.
    and     w5, w5, #1              // Add 1 to the status value stored in w5.
    cbz     w5, _UEFI               // Compare w5 with 0, if w5 == 0 goto offset 0x18

_Linux:
    b       _LinuxStart            // if w5 != 0, continue booting linux kernel.

_UEFI:
    adr     x4, _KernelHead         // Store kernel head address in x4.
    ldr     x5, _KernelSize         // Store kernel size in x5.
    add     x4, x4, x5              // Add kernel base + kernel size, store value in x4.
    ldr     x5, _StackBase          // Store FD Base in x5.
    ldr     x6, _StackSize          // Store FD Size in x6.
    bl      _CopyLoop               // Copy UEFI FD to stack region.
    adr     x0, _ShellCodeEnd       // Store address of ShellCode end to x0.
    adr     x1, _ShellCodeEnd       // Store address of ShellCode end to x1.
    ldr     x5, _StackBase          // Store stack base address in x5.
    br      x5                      // Jump to UEFI.

_CopyLoop:
    ldp     x2, x3, [x4], #0x10     // Save value at [x4](pointer) to x2 and x3, then x4 add 16.
    stp     x2, x3, [x5], #0x10     // Save value in x2 and x3 to [x5](pointer), then x5 add 16
    subs    x6, x6, #0x10           // x6 - 16, if , set CPSR register to 0.
    b.ne    _CopyLoop               // Check CPSR, if CPSR != 0, jump back to _CopyLoop.
    ret                             // Return when finish.

_Dead:
    b       _Dead                   // We should never get here.

    nop                             // Unknown
    nop                             // Unknown

_ShellCodeEnd:

.text
.align 4
/* Do not remove the last line  */
