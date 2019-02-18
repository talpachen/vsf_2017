if("${VSFHAL_CPU_SERIES}" STREQUAL "cortex-m0")
  vsf_compile_definitions(
    ARM_MATH_CM0
  )
  add_subdirectory(TARGET_CORTEX_M)
elseif("${VSFHAL_CPU_SERIES}" STREQUAL "cortex-m0plus")
  vsf_compile_definitions(
    ARM_MATH_CM0PLUS
  )
  add_subdirectory(TARGET_CORTEX_M)
elseif("${VSFHAL_CPU_SERIES}" STREQUAL "cortex-m3")
  vsf_compile_definitions(
    ARM_MATH_CM3
  )
  add_subdirectory(TARGET_CORTEX_M)
elseif("${VSFHAL_CPU_SERIES}" STREQUAL "cortex-m4")
  vsf_compile_definitions(
    ARM_MATH_CM4
  )
  add_subdirectory(TARGET_CORTEX_M)
elseif("${VSFHAL_CPU_SERIES}" STREQUAL "cortex-m7")
  vsf_compile_definitions(
    ARM_MATH_CM7
  )
  add_subdirectory(TARGET_CORTEX_M)
#elseif("${VSFHAL_CPU_SERIES}" STREQUAL "cortex-m23")
#elseif("${VSFHAL_CPU_SERIES}" STREQUAL "cortex-m33")
#elseif("${VSFHAL_CPU_SERIES}" STREQUAL "cortex-m33+nodsp")
else()
  message(FATAL_ERROR "${VSFHAL_CPU_SERIES} not supported")
# add_subdirectory(TARGET_CORTEX_A)
endif()
