cmake_minimum_required(VERSION 3.8.2)

set(VSFHAL_ARCH arm)
set(VSFHAL_CPU cmem7)
set(VSFHAL_CPU_SERIES cortex-m3)

set(CONFIG_DEBUG_OPTIMIZATIONS 1)

include(../../../../vsf.cmake NO_POLICY_SCOPE)
project(NONE)


vsf_compile_definitions(
    FLASH_LOAD_OFFSET=0x08000000
    BOARD_TYPE_TEST_CMEM7
)

vsf_include_directories(../../config)
vsf_include_directories(../../source)

target_sources(app PRIVATE ../../source/usrapp.c)
