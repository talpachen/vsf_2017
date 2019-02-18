cmake_minimum_required(VERSION 3.8.2)

# Configure
set(VSFHAL_ARCH arm)
set(VSFHAL_CPU cmem7)
set(VSFHAL_CPU_SERIES cortex-m3)

set(VSFCOMPO_DYNPOOL 1)


set(CONFIG_DEBUG_OPTIMIZATIONS 1)

# Common Include
include(../../vsf.cmake NO_POLICY_SCOPE)

# Project Name
project(cmake_example_cmem7)


vsf_compile_definitions(
    FLASH_LOAD_OFFSET=0x08000000
    BOARD_TYPE_TEST_CMEM7
)

add_subdirectory(config)
vsf_include_directories(.)

target_sources(app PRIVATE usrapp.c)