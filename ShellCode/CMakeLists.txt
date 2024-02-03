cmake_minimum_required(VERSION 3.27)
project(ShellCode ASM)

file(GLOB ASM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ShellCode.*.S")

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    # Windows
    # MinGW compiler download link:
    #   https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
    set(CROSS_COMPILE "E:/aarch64-gcc/aarch64-none-elf/bin/")
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    # Linux
    set(CROSS_COMPILE "aarch64-linux-gnu-")
else ()
    message("build on ${CMAKE_HOST_SYSTEM_NAME} is not supported")
endif ()

set(CMAKE_AS ${CROSS_COMPILE}as)
set(CMAKE_OBJCOPY ${CROSS_COMPILE}objcopy)

foreach (CURRENT_ASM_SOURCE_SUFFIX ${ASM_SOURCES})
    string(REGEX REPLACE "(ShellCode.*)\\.S" "\\1" CURRENT_ASM_SOURCE ${CURRENT_ASM_SOURCE_SUFFIX})
    add_custom_target(
        ${CURRENT_ASM_SOURCE}.elf ALL
        ${CMAKE_AS}
        ${CMAKE_CURRENT_SOURCE_DIR}/${CURRENT_ASM_SOURCE}.S
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        -o ${CURRENT_ASM_SOURCE}.elf
    )

    add_custom_target(
        ${CURRENT_ASM_SOURCE}.bin ALL
        ${CMAKE_OBJCOPY} -O binary
        ${CURRENT_ASM_SOURCE}.elf
        ${CURRENT_ASM_SOURCE}.bin
        DEPENDS ${CURRENT_ASM_SOURCE}.elf
    )
endforeach ()