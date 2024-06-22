/** @file
 * DualBoot Kernel Patcher Header File.
 *
 *  Copyright (c) 2021-2024 The DuoWoa authors. All rights reserved.
 *  MIT License
 *
 */

#ifndef LINUX_KERNEL_PATCHER_PATCHER_H
#define LINUX_KERNEL_PATCHER_PATCHER_H

#include <stdint.h>

//
//  Config info read from config file and set to kernel.
//
typedef struct {
    uint64_t StackBase;
    uint64_t StackSize;
} Config, *pConfig;

//
// Store some file information and file buffer.
//
typedef struct {
    uint8_t *fileBuffer;
    size_t fileSize;
    const char *filePath;
} FileContent, *pFileContent;

size_t get_file_size(FileContent *fileContent);

uint8_t *read_file_content(FileContent *fileContent);

int write_file_content(pFileContent fileContent);

int parse_config(FileContent *fileContent, pConfig config);

uint8_t *PatchKernel(pFileContent kernel, pFileContent uefi, pFileContent shellCode,
                     pFileContent patchedKernel, pConfig config);

#endif //LINUX_KERNEL_PATCHER_PATCHER_H
