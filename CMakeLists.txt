cmake_minimum_required(VERSION 3.27)
project(DualBootKernelPatcher C)

set(CMAKE_C_STANDARD 11)

# Compile the patcher.
add_executable(DualBootKernelPatcher patcher.c)

# Compile Shell Codes with aarch64 gcc.
add_subdirectory(ShellCode)