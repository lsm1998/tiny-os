# 是否是macOS系统
if(APPLE)
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR x86)

    set(CMAKE_C_COMPILER x86_64-elf-gcc)
    set(CMAKE_ASM_COMPILER x86_64-elf-gcc)

    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()
