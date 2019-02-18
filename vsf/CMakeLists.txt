project(VSF-Kernel)
enable_language(C ASM)

#check_c_compiler_flag("" toolchain_is_ok)
#assert(toolchain_is_ok "The toolchain is unable to build a dummy C file. See CMakeError.log.")

set(CMAKE_EXECUTABLE_SUFFIX .elf)

if(NOT PROPERTY_LINKER_SCRIPT_DEFINES)
  set_property(GLOBAL PROPERTY PROPERTY_LINKER_SCRIPT_DEFINES -D__GCC_LINKER_CMD__)
endif()

define_property(GLOBAL PROPERTY PROPERTY_OUTPUT_FORMAT BRIEF_DOCS " " FULL_DOCS " ")
set_property(   GLOBAL PROPERTY PROPERTY_OUTPUT_FORMAT elf32-little${VSFHAL_ARCH})

add_library(vsf_interface INTERFACE)

vsf_library_named(vsf)

vsf_include_directories(
  .
)

if(LIBC_INCLUDE_DIR)
  vsf_include_directories(${LIBC_INCLUDE_DIR})
  EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  string(REGEX REPLACE "\n$" "" GCC_VERSION "${GCC_VERSION}")
  vsf_include_directories(${LIBC_INCLUDE_DIR}/../../lib/gcc/arm-none-eabi/${GCC_VERSION}/include)
endif()

vsf_sources(vsfmain.c)

#vsf_compile_definitions()

if(NOT CONFIG_NO_OPTIMIZATIONS)
vsf_compile_definitions(
  _FORTIFY_SOURCE=2
)
endif()


set_ifndef(OPTIMIZE_FOR_NO_OPTIMIZATIONS_FLAG "-O0")
set_ifndef(OPTIMIZE_FOR_DEBUG_FLAG            "-Og")
set_ifndef(OPTIMIZE_FOR_SIZE_FLAG             "-Os")
set_ifndef(OPTIMIZE_FOR_SPEED_FLAG            "-O2")

if(CONFIG_NO_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_NO_OPTIMIZATIONS_FLAG})
elseif(CONFIG_DEBUG_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_DEBUG_FLAG})
elseif(CONFIG_SPEED_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_SPEED_FLAG})
elseif(CONFIG_SIZE_OPTIMIZATIONS)
  set(OPTIMIZATION_FLAG ${OPTIMIZE_FOR_SIZE_FLAG})
else()
  assert(0 "Unreachable code. Expected optimization level to have been chosen. See misc/Kconfig.")
endif()

if(NOT CONFIG_NATIVE_APPLICATION)
  set(NOSTDINC_F -nostdinc)

  vsf_ld_options(
    -nostartfiles
    -nodefaultlibs
    -nostdlib
    -static
    -no-pie
  )
endif()

vsf_compile_options(
  ${OPTIMIZATION_FLAG} # Usually -Os
  -g # TODO: build configuration enough?
  -Wall
  -Wformat
  -Wformat-security
  -Wno-format-zero-length
  #-imacros ${AUTOCONF_H}
  -ffreestanding
  -Wno-main
  ${NOSTDINC_F}
  ${TOOLCHAIN_C_FLAGS}
)

vsf_compile_options(
  $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
  $<$<COMPILE_LANGUAGE:CXX>:-fcheck-new>
  $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
  $<$<COMPILE_LANGUAGE:CXX>:-fdata-sections>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>

  $<$<COMPILE_LANGUAGE:ASM>:-xassembler-with-cpp>
  $<$<COMPILE_LANGUAGE:ASM>:-D_ASMLANGUAGE>
)

# ==========================================================================
#
# cmake -DW=... settings
#
# W=1 - warnings that may be relevant and does not occur too often
# W=2 - warnings that occur quite often but may still be relevant
# W=3 - the more obscure warnings, can most likely be ignored
# ==========================================================================
if(W MATCHES "1")
  vsf_compile_options(
    -Wextra
    -Wunused
    -Wno-unused-parameter
    -Wmissing-declarations
    -Wmissing-format-attribute
    -Wold-style-definition
  )
  vsf_cc_option(
    -Wmissing-prototypes
    -Wmissing-include-dirs
    -Wunused-but-set-variable
    -Wno-missing-field-initializers
  )
endif()

if(W MATCHES "2")
  vsf_compile_options(
    -Waggregate-return
    -Wcast-align
    -Wdisabled-optimization
    -Wnested-externs
    -Wshadow
  )
  vsf_cc_option(
    -Wlogical-op
    -Wmissing-field-initializers
  )
endif()

if(W MATCHES "3")
  vsf_compile_options(
    -Wbad-function-cast
    -Wcast-qual
    -Wconversion
    -Wpacked
    -Wpadded
    -Wpointer-arith
    -Wredundant-decls
    -Wswitch-default
  )
  vsf_cc_option(
    -Wpacked-bitfield-compat
    -Wvla
  )
endif()


# Allow the user to inject options when calling cmake, e.g.
# 'cmake -DEXTRA_CFLAGS="-Werror -Wno-deprecated-declarations" ..'
separate_arguments(EXTRA_CPPFLAGS_AS_LIST UNIX_COMMAND ${EXTRA_CPPFLAGS})
separate_arguments(EXTRA_LDFLAGS_AS_LIST  UNIX_COMMAND ${EXTRA_LDFLAGS})
separate_arguments(EXTRA_CFLAGS_AS_LIST   UNIX_COMMAND ${EXTRA_CFLAGS})
separate_arguments(EXTRA_CXXFLAGS_AS_LIST UNIX_COMMAND ${EXTRA_CXXFLAGS})
separate_arguments(EXTRA_AFLAGS_AS_LIST   UNIX_COMMAND ${EXTRA_AFLAGS})
if(EXTRA_CPPFLAGS)
  vsf_compile_definitions(${EXTRA_CPPFLAGS_AS_LIST})
endif()
if(EXTRA_LDFLAGS)
  vsf_link_libraries(${EXTRA_LDFLAGS_AS_LIST})
endif()
if(EXTRA_CFLAGS)
  foreach(F ${EXTRA_CFLAGS_AS_LIST})
    vsf_compile_options($<$<COMPILE_LANGUAGE:C>:${F}>)
  endforeach()
endif()
if(EXTRA_CXXFLAGS)
  foreach(F ${EXTRA_CXXFLAGS_AS_LIST})
    vsf_compile_options($<$<COMPILE_LANGUAGE:CXX>:${F}>)
  endforeach()
endif()
if(EXTRA_AFLAGS)
  foreach(F ${EXTRA_AFLAGS_AS_LIST})
    vsf_compile_options($<$<COMPILE_LANGUAGE:ASM>:${F}>)
  endforeach()
endif()

if(CONFIG_READABLE_ASM)
  vsf_cc_option(-fno-reorder-blocks)
  vsf_cc_option(-fno-ipa-cp-clone)
  vsf_cc_option(-fno-partial-inlining)
endif()

vsf_cc_option(-fno-asynchronous-unwind-tables)
vsf_cc_option(-fno-pie)
vsf_cc_option(-fno-pic)
vsf_cc_option(-fno-strict-overflow)
vsf_cc_option(-Wno-pointer-sign)

vsf_compile_options_ifdef(CONFIG_STACK_CANARIES -fstack-protector-all)

if(CONFIG_OVERRIDE_FRAME_POINTER_DEFAULT)
  if(CONFIG_OMIT_FRAME_POINTER)
    vsf_cc_option(-fomit-frame-pointer)
  else()
    vsf_cc_option(-fno-omit-frame-pointer)
  endif()
endif()

vsf_cc_option(
  -Wno-unused-but-set-variable
  -fno-reorder-functions
)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  vsf_cc_option(
    #FIXME: need to fix all of those
    -Wno-sometimes-uninitialized
    -Wno-shift-overflow
    -Wno-missing-braces
    -Wno-self-assign
    -Wno-address-of-packed-member
    -Wno-unused-function
    -Wno-initializer-overrides
    -Wno-section
    -Wno-unknown-warning-option
    -Wno-unused-variable
    -Wno-format-invalid-specifier
    -Wno-gnu
    # comparison of unsigned expression < 0 is always false
    -Wno-tautological-compare
  )
else() # GCC assumed
  vsf_cc_option(
    -Wno-unused-but-set-variable
    -fno-reorder-functions
  )

  if(NOT ${VSF_TOOLCHAIN_VARIANT} STREQUAL "xcc")
    vsf_cc_option(-fno-defer-pop)
  endif()
endif()

vsf_cc_option_ifdef(CONFIG_STACK_USAGE            -fstack-usage)

vsf_system_include_directories(${NOSTDINC})

# Force an error when things like SYS_INIT(foo, ...) occur with a missing header.
vsf_cc_option(-Werror=implicit-int)

# Prohibit void pointer arithmetic. Illegal in C99
vsf_cc_option(-Wpointer-arith)

# Prohibit date/time macros, which would make the build non-deterministic
# vsf_cc_option(-Werror=date-time)

set_ifndef(LINKERFLAGPREFIX -Wl)

if(NOT CONFIG_NATIVE_APPLICATION)
vsf_ld_options(
  ${LINKERFLAGPREFIX},-X
  ${LINKERFLAGPREFIX},-N
)
endif()

vsf_ld_options(
  ${LINKERFLAGPREFIX},--gc-sections
  ${LINKERFLAGPREFIX},--build-id=none
)

message(STATUS ${CMAKE_BINARY_DIR})
message(STATUS ${CMAKE_CACHEFILE_DIR})
message(STATUS ${CMAKE_HOME_DIRECTORY})
message(STATUS ${CMAKE_SOURCE_DIR})
message(STATUS ${PROJECT_SOURCE_DIR})


set(LINKER_SCRIPT ${APPLICATION_SOURCE_DIR}/${CONFIG_CUSTOM_LINKER_SCRIPT})

add_subdirectory(compiler)
add_subdirectory(hal)
add_subdirectory(framework)
add_subdirectory(component)
