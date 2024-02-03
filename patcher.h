//
//  Kernel Patcher Header File.
//

#ifndef LINUX_KERNEL_PATCHER_PATCHER_H
#define LINUX_KERNEL_PATCHER_PATCHER_H

#include <stdint.h>

typedef struct {
    uint64_t StackBase;
    uint64_t StackSize;
} Stack, *pStack;

size_t get_file_size(char *filePath);
void parse_config(char* filePath, pStack stack);
uint8_t *PatchKernel(uint8_t *kernelBuffer, size_t kernelBufferSize, uint8_t *uefiBuffer, size_t uefiBufferSize,
                     uint8_t *shellCodeBuffer, size_t shellCodeSize, pStack stack);

#endif //LINUX_KERNEL_PATCHER_PATCHER_H
