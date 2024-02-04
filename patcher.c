#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "patcher.h"

int main(int argc, char *argv[]) {
    // Print hello message
    printf("WOA-msmnile DualBoot Kernel Image Patcher v1.0.0.0 !\n");
    printf("Copyright (c) 2021-2024 The DuoWoA authors\n\n");
    if (argc != 6) {
        // Print usage if arg numbers not meet.
        printf("Usage: <Kernel Image to Patch> <UEFI FD Image> <Patched Kernel Image Destination> "
               "<Config File> <Shell Code File>\n");
        return -EINVAL;
    }

    // Get file paths
    char *originImage = argv[1];
    char *uefiImage = argv[2];
    char *outputImage = argv[3];
    char *config = argv[4];
    char *shellCode = argv[5];

    printf("Patching %s with %s and saving to %s...\n\n", originImage, uefiImage, outputImage);

    // Read buffer from old kernel.
    size_t originImageSize = get_file_size(originImage);
    if (originImageSize == 0) {
        printf("Error: Please check the kernel you provided.\n");
        return -EINVAL;
    }
    void *originImageBuffer = malloc(originImageSize);
    FILE *pOriginImageFile = fopen(originImage, "rb");
    fread(originImageBuffer, originImageSize, 1, pOriginImageFile);
    fclose(pOriginImageFile);

    // Read buffer from uefi image.
    size_t uefiImageSize = get_file_size(uefiImage);
    if (uefiImageSize == 0) {
        printf("Error: Please check the uefi fd you provided.\n");
        return -EINVAL;
    }
    void *uefiImageBuffer = malloc(uefiImageSize);
    FILE *pUefiImageFile = fopen(uefiImage, "rb");
    fread(uefiImageBuffer, uefiImageSize, 1, pUefiImageFile);
    fclose(pUefiImageFile);

    // Parse config file.
    Stack stack = {0};
    if (parse_config(config, &stack)) {
        printf("Error: Please check config your provided.\n");
        return -EINVAL;
    }

    // Get ShellCode buffer.
    size_t shellCodeSize = get_file_size(shellCode);
    void *shellCodeBuffer = malloc(shellCodeSize);
    FILE *pShellCodeFile = fopen(shellCode, "rb");
    if (pShellCodeFile == NULL) {
        printf("Error: Please check the shell code binary you provided.\n");
        return -EINVAL;
    }
    fread(shellCodeBuffer, shellCodeSize, 1, pShellCodeFile);
    fclose(pShellCodeFile);

    // Patch Kernel.
    size_t outputImageSize = originImageSize + uefiImageSize;
    uint8_t *outputImageBuffer = PatchKernel(originImageBuffer, originImageSize, uefiImageBuffer, uefiImageSize,
                                             shellCodeBuffer, shellCodeSize, &stack);

    // Output buffer to new kernel.
    if (outputImageBuffer != NULL) {
        FILE *pOutputImageFile = fopen(outputImage, "wb");
        fwrite(outputImageBuffer, outputImageSize, 1, pOutputImageFile);
        fclose(pOriginImageFile);
    } else {
        printf("Error Patching Kernel.\n");
        return -EINVAL;
    }

    // Free buffers.
    free(shellCodeBuffer);
    free(originImageBuffer);
    free(uefiImageBuffer);
    free(outputImageBuffer);

    // Print end message.
    printf("Image successfully patched.\n");
    printf("Please find the newly made kernel image at %s\n", outputImage);
    return 0;
}

size_t get_file_size(char *filePath) {
    FILE *pFile = fopen(filePath, "r");
    if (pFile == NULL)
        return 0;
    fseek(pFile, 0, SEEK_END);
    size_t len = ftell(pFile);
    fclose(pFile);
    return len;
}

int parse_config(char *filePath, pStack stack) {
    FILE *pConfigFile = fopen(filePath, "r");
    char key[256];
    uint32_t value = 0;

    // Check File Size
    if (!get_file_size(filePath))
        return -1;

    // Parse
    while (fscanf(pConfigFile, "%[^=]=%x\n", key, &value) != EOF) {
        if (strcmp(key, "StackBase") == 0) {
            stack->StackBase = value;
        } else if (strcmp(key, "StackSize") == 0) {
            stack->StackSize = value;
        }
    }

    fclose(pConfigFile);
    return 0;
}

uint8_t *PatchKernel(uint8_t *kernelBuffer, size_t kernelBufferSize, uint8_t *uefiBuffer, size_t uefiBufferSize,
                     uint8_t *shellCodeBuffer, size_t shellCodeSize, pStack stack) {

    // Check if kernel has UNCOMPRESSED_IMG Header.
    char kernelHeader[0x11] = {0};
    memcpy(kernelHeader, kernelBuffer, 0x10);
    if (strcmp(kernelHeader, "UNCOMPRESSED_IMG") == 0) {
        printf("Kernel has header, removing it...\n");
        kernelBuffer += 0x14;
        kernelBufferSize -= 0x14;
    }

    // Allocate patched image buffer.
    size_t patchedKernelBufferSize = kernelBufferSize + uefiBufferSize;
    uint8_t *patchedKernelBuffer = malloc(patchedKernelBufferSize);

    // Copy two buffers into patchedBuffer.
    memcpy(patchedKernelBuffer, kernelBuffer, kernelBufferSize);
    memcpy(patchedKernelBuffer + kernelBufferSize, uefiBuffer, uefiBufferSize);

    if (uefiBufferSize > 0x40 && strcmp((char *) (shellCodeBuffer + 8), "SHLLCOD") != 0) {
        printf("Error: shell code binary format not recognize.\n");
        return NULL;
    }

    // Determine the loading offset of the kernel first,
    // we are either going to find a b instruction on the
    // first instruction or the second one. First is problematic,
    // second is fine.

    // 0x14 is AArch64 b opcode
    if (patchedKernelBuffer[3] == 0x14) {
        // We have a branch instruction first, we need to fix things a bit.

        // First start by getting the actual value of the branch addr offset.
        uint8_t offsetInstructionBuffer[] = {patchedKernelBuffer[0], patchedKernelBuffer[1], patchedKernelBuffer[2], 0};
        uint32_t *offsetInstruction = (uint32_t *) offsetInstructionBuffer;

        // Now subtract 1 instruction because we'll move this to the second instruction of the kernel header.
        (*offsetInstruction)--;

        // Convert back into an actual value that is usable as a b instruction
        offsetInstructionBuffer[3] = 0x14; // Useless but just for our sanity :)
        // Now write the instruction back into the kernel (instr 2)
        patchedKernelBuffer[4] = offsetInstructionBuffer[0];
        patchedKernelBuffer[5] = offsetInstructionBuffer[1];
        patchedKernelBuffer[6] = offsetInstructionBuffer[2];
        patchedKernelBuffer[7] = 0x14;

    } else if (patchedKernelBuffer[7] != 0x14) {
        // There is no branch instruction!
        printf("Error: Invalid Kernel Image. Branch instruction not found within first two instruction slots.\n");
        return NULL;
    } else {
        return NULL;
    }

    // Alright, our kernel image has a compatible branch instruction, let's start.

    // First, add the jump to our code on instr 1
    // This directly jump right after the kernel header (there's enough headroom here)
    patchedKernelBuffer[0] = 0x10;
    patchedKernelBuffer[1] = 0;
    patchedKernelBuffer[2] = 0;
    patchedKernelBuffer[3] = 0x14;

    // Now we need to fill in the stack base of our firmware
    // Stack Base: 0x00000000 9FC00000 (64 bit!)
    patchedKernelBuffer[0x20] = stack->StackBase >> 0 & 0xFF;
    patchedKernelBuffer[0x21] = stack->StackBase >> 8 & 0xFF;
    patchedKernelBuffer[0x22] = stack->StackBase >> 16 & 0xFF;
    patchedKernelBuffer[0x23] = stack->StackBase >> 24 & 0xFF;
    patchedKernelBuffer[0x24] = stack->StackBase >> 32 & 0xFF;
    patchedKernelBuffer[0x25] = stack->StackBase >> 40 & 0xFF;
    patchedKernelBuffer[0x26] = stack->StackBase >> 48 & 0xFF;
    patchedKernelBuffer[0x27] = stack->StackBase >> 56 & 0xFF;

    // Then we need to fill in the stack size of our firmware
    // Stack Base: 0x00000000 00300000 (64 bit!)
    patchedKernelBuffer[0x28] = stack->StackSize >> 0 & 0xFF;
    patchedKernelBuffer[0x29] = stack->StackSize >> 8 & 0xFF;
    patchedKernelBuffer[0x2A] = stack->StackSize >> 16 & 0xFF;
    patchedKernelBuffer[0x2B] = stack->StackSize >> 24 & 0xFF;
    patchedKernelBuffer[0x2C] = stack->StackSize >> 32 & 0xFF;
    patchedKernelBuffer[0x2D] = stack->StackSize >> 40 & 0xFF;
    patchedKernelBuffer[0x2E] = stack->StackSize >> 48 & 0xFF;
    patchedKernelBuffer[0x2F] = stack->StackSize >> 56 & 0xFF;

    // Finally, we add in the total kernel image size because we need to jump over!
    patchedKernelBuffer[0x30] = kernelBufferSize >> 0 & 0xFF;
    patchedKernelBuffer[0x31] = kernelBufferSize >> 8 & 0xFF;
    patchedKernelBuffer[0x32] = kernelBufferSize >> 16 & 0xFF;
    patchedKernelBuffer[0x33] = kernelBufferSize >> 24 & 0xFF;
    patchedKernelBuffer[0x34] = kernelBufferSize >> 32 & 0xFF;
    patchedKernelBuffer[0x35] = kernelBufferSize >> 40 & 0xFF;
    patchedKernelBuffer[0x36] = kernelBufferSize >> 48 & 0xFF;
    patchedKernelBuffer[0x37] = kernelBufferSize >> 56 & 0xFF;

    // Now our header is fully patched, let's add a tiny bit
    // of code as well to decide what to do.
    // Ignore 0x40 Dummy Header in shellCodeBuffer, so we add 0x40 offset after it.
    memcpy(patchedKernelBuffer + 0x40, shellCodeBuffer + 0x40, shellCodeSize - 0x40);

    // And that's it, the user now can append executable code right after the kernel,
    // and upon closing up the device said code will run at boot. Have fun!
    return patchedKernelBuffer;
}