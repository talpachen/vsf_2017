cmake_minimum_required(VERSION 3.8.2)

define_property(GLOBAL PROPERTY VSF_LIBS
    BRIEF_DOCS "Global list of all VSF CMake libs that should be linked in"
    FULL_DOCS  "Global list of all VSF CMake libs that should be linked in.
vsf_library() appends libs to this list.")
set_property(GLOBAL PROPERTY VSF_LIBS "")

define_property(GLOBAL PROPERTY VSF_INTERFACE_LIBS
    BRIEF_DOCS "Global list of all VSF interface libs that should be linked in."
    FULL_DOCS  "Global list of all VSF interface libs that should be linked in.
vsf_interface_library_named() appends libs to this list.")
set_property(GLOBAL PROPERTY VSF_INTERFACE_LIBS "")

define_property(GLOBAL PROPERTY GENERATED_KERNEL_OBJECT_FILES
  BRIEF_DOCS "Object files that are generated after VSF has been linked once."
  FULL_DOCS "\
Object files that are generated after VSF has been linked once.\
May include mmu tables, etc."
  )
set_property(GLOBAL PROPERTY GENERATED_KERNEL_OBJECT_FILES "")

define_property(GLOBAL PROPERTY GENERATED_KERNEL_SOURCE_FILES
  BRIEF_DOCS "Source files that are generated after VSF has been linked once."
  FULL_DOCS "\
Object files that are generated after VSF has been linked once.\
May include isr_tables.c etc."
  )
set_property(GLOBAL PROPERTY GENERATED_KERNEL_SOURCE_FILES "")


if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "Source directory equals build directory.\
    Please specify a build directory.")
endif()

# Clean ${CMAKE_BINARY_DIR}
file(GLOB build_dir_contents ${CMAKE_BINARY_DIR}/*)
foreach(file ${build_dir_contents})
  if (EXISTS ${file})
     file(REMOVE_RECURSE ${file})
  endif()
endforeach(file)



set(VSF_BASE       ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "VSF Base Directory")

set(VSF_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/vsf CACHE PATH "VSF Binary Directory")

set(APP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Application Source Directory")
set(APP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Application Binary Directory")


#[[
extensions
]]
macro(vsf_library)
  vsf_library_get_current_dir_lib_name(lib_name)
  vsf_library_named(${lib_name})
endmacro()

macro(vsf_library_get_current_dir_lib_name lib_name)
  file(RELATIVE_PATH name ${VSF_BASE} ${CMAKE_CURRENT_LIST_FILE})
  get_filename_component(name ${name} DIRECTORY)
  string(REGEX REPLACE "/" "__" name ${name})
  set(${lib_name} ${name})
endmacro()

macro(vsf_library_named name)
  set(VSF_CURRENT_LIBRARY ${name})
  add_library(${name} STATIC "")
  vsf_append_cmake_library(${name})
  target_link_libraries(${name} PUBLIC vsf_interface)
endmacro()

function(vsf_link_interface interface)
  target_link_libraries(${interface} INTERFACE vsf_interface)
endfunction()

#
# vsf_library versions of normal CMake target_<func> functions
#
function(vsf_library_sources source)
  target_sources(${VSF_CURRENT_LIBRARY} PRIVATE ${source} ${ARGN})
endfunction()

function(vsf_library_include_directories)
  target_include_directories(${VSF_CURRENT_LIBRARY} PRIVATE ${ARGN})
endfunction()

function(vsf_library_link_libraries item)
  target_link_libraries(${VSF_CURRENT_LIBRARY} PUBLIC ${item} ${ARGN})
endfunction()

function(vsf_library_compile_definitions item)
  target_compile_definitions(${VSF_CURRENT_LIBRARY} PRIVATE ${item} ${ARGN})
endfunction()

function(vsf_library_compile_options item)
  # The compiler is relied upon for sane behaviour when flags are in
  # conflict. Compilers generally give precedence to flags given late
  # on the command line. So to ensure that vsf_library_* flags are
  # placed late on the command line we create a dummy interface
  # library and link with it to obtain the flags.
  #
  # Linking with a dummy interface library will place flags later on
  # the command line than the the flags from vsf_interface because
  # vsf_interface will be the first interface library that flags
  # are taken from.

  string(RANDOM random)
  set(lib_name options_interface_lib_${random})

  add_library(           ${lib_name} INTERFACE)
  target_compile_options(${lib_name} INTERFACE ${item} ${ARGN})

  target_link_libraries(${VSF_CURRENT_LIBRARY} PRIVATE ${lib_name})
endfunction()

function(vsf_library_cc_option)
  foreach(option ${ARGV})
    string(MAKE_C_IDENTIFIER check${option} check)
    vsf_check_compiler_flag(C ${option} ${check})

    if(${check})
      vsf_library_compile_options(${option})
    endif()
  endforeach()
endfunction()

# Add the existing CMake library 'library' to the global list of
# VSF CMake libraries. This is done automatically by the
# constructor but must called explicitly on CMake libraries that do
# not use a vsf library constructor, but have source files that
# need to be included in the build.
function(vsf_append_cmake_library library)
  set_property(GLOBAL APPEND PROPERTY VSF_LIBS ${library})
endfunction()



function(vsf_append_cmake_library library)
  set_property(GLOBAL APPEND PROPERTY VSF_LIBS ${library})
endfunction()


function(add_subdirectory_ifdef feature_toggle dir)
  if(${${feature_toggle}})
    add_subdirectory(${dir})
  endif()
endfunction()

function(target_sources_ifdef feature_toggle target scope item)
  if(${${feature_toggle}})
    target_sources(${target} ${scope} ${item} ${ARGN})
  endif()
endfunction()

function(target_compile_definitions_ifdef feature_toggle target scope item)
  if(${${feature_toggle}})
    target_compile_definitions(${target} ${scope} ${item} ${ARGN})
  endif()
endfunction()

function(target_include_directories_ifdef feature_toggle target scope item)
  if(${${feature_toggle}})
    target_include_directories(${target} ${scope} ${item} ${ARGN})
  endif()
endfunction()

function(target_link_libraries_ifdef feature_toggle target item)
  if(${${feature_toggle}})
    target_link_libraries(${target} ${item} ${ARGN})
  endif()
endfunction()

function(add_compile_option_ifdef feature_toggle option)
  if(${${feature_toggle}})
    add_compile_options(${option})
  endif()
endfunction()

function(target_compile_option_ifdef feature_toggle target scope option)
  if(${feature_toggle})
    target_compile_options(${target} ${scope} ${option})
  endif()
endfunction()

function(target_cc_option_ifdef feature_toggle target scope option)
  if(${feature_toggle})
    target_cc_option(${target} ${scope} ${option})
  endif()
endfunction()

function(vsf_library_sources_ifdef feature_toggle source)
  if(${${feature_toggle}})
    vsf_library_sources(${source} ${ARGN})
  endif()
endfunction()

function(vsf_sources_ifdef feature_toggle)
  if(${${feature_toggle}})
    vsf_sources(${ARGN})
  endif()
endfunction()

function(vsf_sources_ifndef feature_toggle)
   if(NOT ${feature_toggle})
    vsf_sources(${ARGN})
  endif()
endfunction()

function(vsf_cc_option_ifdef feature_toggle)
  if(${${feature_toggle}})
    vsf_cc_option(${ARGN})
  endif()
endfunction()

function(vsf_ld_option_ifdef feature_toggle)
  if(${${feature_toggle}})
    vsf_ld_options(${ARGN})
  endif()
endfunction()

function(vsf_link_libraries_ifdef feature_toggle)
  if(${${feature_toggle}})
    vsf_link_libraries(${ARGN})
  endif()
endfunction()

function(vsf_compile_options_ifdef feature_toggle)
  if(${${feature_toggle}})
    vsf_compile_options(${ARGN})
  endif()
endfunction()

function(vsf_compile_definitions_ifdef feature_toggle)
  if(${${feature_toggle}})
    vsf_compile_definitions(${ARGN})
  endif()
endfunction()

function(vsf_include_directories_ifdef feature_toggle)
  if(${${feature_toggle}})
    vsf_include_directories(${ARGN})
  endif()
endfunction()

function(vsf_library_compile_definitions_ifdef feature_toggle item)
  if(${${feature_toggle}})
    vsf_library_compile_definitions(${item} ${ARGN})
  endif()
endfunction()

function(vsf_library_compile_options_ifdef feature_toggle item)
  if(${${feature_toggle}})
    vsf_library_compile_options(${item} ${ARGN})
  endif()
endfunction()

function(vsf_link_interface_ifdef feature_toggle interface)
  if(${${feature_toggle}})
    target_link_libraries(${interface} INTERFACE vsf_interface)
  endif()
endfunction()

function(vsf_library_link_libraries_ifdef feature_toggle item)
  if(${${feature_toggle}})
     vsf_library_link_libraries(${item})
  endif()
endfunction()

macro(list_append_ifdef feature_toggle list)
  if(${${feature_toggle}})
    list(APPEND ${list} ${ARGN})
  endif()
endmacro()

function(set_ifndef variable value)
  if(NOT ${variable})
    set(${variable} ${value} ${ARGN} PARENT_SCOPE)
  endif()
endfunction()

function(target_cc_option_ifndef feature_toggle target scope option)
  if(NOT ${feature_toggle})
    target_cc_option(${target} ${scope} ${option})
  endif()
endfunction()

function(vsf_cc_option_ifndef feature_toggle)
  if(NOT ${feature_toggle})
    vsf_cc_option(${ARGN})
  endif()
endfunction()

function(vsf_compile_options_ifndef feature_toggle)
  if(NOT ${feature_toggle})
    vsf_compile_options(${ARGN})
  endif()
endfunction()

function(check_compiler_flag lang option ok)
  if(NOT DEFINED CMAKE_REQUIRED_QUIET)
    set(CMAKE_REQUIRED_QUIET 1)
  endif()

  string(MAKE_C_IDENTIFIER
    check${option}_${lang}_${CMAKE_REQUIRED_FLAGS}
    ${ok}
    )

  if(0) # FIXME
    if(${lang} STREQUAL C)
      check_c_compiler_flag("${option}" ${${ok}})
    else()
      check_cxx_compiler_flag("${option}" ${${ok}})
    endif()
  else()
  set(ok 1)
  endif()

  if(${${${ok}}})
    set(ret 1)
  else()
    set(ret 0)
  endif()

  set(${ok} ${ret} PARENT_SCOPE)
endfunction()

function(vsf_check_compiler_flag lang option check)
  # Locate the cache
  set_ifndef(
    VSF_TOOLCHAIN_CAPABILITY_CACHE
    ${USER_CACHE_DIR}/ToolchainCapabilityDatabase.cmake
    )

  # Read the cache
  include(${VSF_TOOLCHAIN_CAPABILITY_CACHE} OPTIONAL)

  # We need to create a unique key wrt. testing the toolchain
  # capability. This key must be a valid C identifier that includes
  # everything that can affect the toolchain test.

  # The 'cacheformat' must be bumped if a bug in the caching mechanism
  # is detected and all old keys must be invalidated.
  set(cacheformat 2)

  set(key_string "")
  set(key_string "${key_string}VSF_TOOLCHAIN_CAPABILITY_CACHE")
  set(key_string "${key_string}cacheformat_")
  set(key_string "${key_string}${cacheformat}_")
  set(key_string "${key_string}${TOOLCHAIN_SIGNATURE}_")
  set(key_string "${key_string}${lang}_")
  set(key_string "${key_string}${option}_")
  set(key_string "${key_string}${CMAKE_REQUIRED_FLAGS}_")

  string(MAKE_C_IDENTIFIER ${key_string} key)

  # Check the cache
  if(DEFINED ${key})
    set(${check} ${${key}} PARENT_SCOPE)
    return()
  endif()

  # Test the flag
  check_compiler_flag(${lang} "${option}" inner_check)

  set(${check} ${inner_check} PARENT_SCOPE)

  # Populate the cache
  file(
    APPEND
    ${VSF_TOOLCHAIN_CAPABILITY_CACHE}
    "set(${key} ${inner_check})\n"
    )
endfunction()

function(target_cc_option target scope option)
  target_cc_option_fallback(${target} ${scope} ${option} "")
endfunction()

# Support an optional second option for when the first option is not
# supported.
function(target_cc_option_fallback target scope option1 option2)
  if(CONFIG_CPLUSPLUS)
    foreach(lang C CXX)
      # For now, we assume that all flags that apply to C/CXX also
      # apply to ASM.
      vsf_check_compiler_flag(${lang} ${option1} check)
      if(${check})
        target_compile_options(${target} ${scope}
          $<$<COMPILE_LANGUAGE:${lang}>:${option1}>
          $<$<COMPILE_LANGUAGE:ASM>:${option1}>
          )
      elseif(option2)
        target_compile_options(${target} ${scope}
          $<$<COMPILE_LANGUAGE:${lang}>:${option2}>
          $<$<COMPILE_LANGUAGE:ASM>:${option2}>
          )
      endif()
    endforeach()
  else()
    vsf_check_compiler_flag(C ${option1} check)
    if(${check})
      target_compile_options(${target} ${scope} ${option1})
    elseif(option2)
      target_compile_options(${target} ${scope} ${option2})
    endif()
  endif()
endfunction()

function(target_ld_options target scope)
  foreach(option ${ARGN})
    string(MAKE_C_IDENTIFIER check${option} check)

    set(SAVED_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${option}")
    vsf_check_compiler_flag(C "" ${check})
    set(CMAKE_REQUIRED_FLAGS ${SAVED_CMAKE_REQUIRED_FLAGS})

    target_link_libraries_ifdef(${check} ${target} ${scope} ${option})
  endforeach()
endfunction()

# https://cmake.org/cmake/help/latest/command/target_sources.html
function(vsf_sources)
  foreach(arg ${ARGV})
    if(IS_ABSOLUTE ${arg})
      set(path ${arg})
    else()
      set(path ${CMAKE_CURRENT_SOURCE_DIR}/${arg})
    endif()

    if(IS_DIRECTORY ${path})
      message(FATAL_ERROR "vsf_sources() was called on a directory")
    endif()

    target_sources(vsf PRIVATE ${path})
  endforeach()
endfunction()

# https://cmake.org/cmake/help/latest/command/target_include_directories.html
function(vsf_include_directories)
  foreach(arg ${ARGV})
    if(IS_ABSOLUTE ${arg})
      set(path ${arg})
    else()
      set(path ${CMAKE_CURRENT_SOURCE_DIR}/${arg})
    endif()
    target_include_directories(vsf_interface INTERFACE ${path})
  endforeach()
endfunction()

# https://cmake.org/cmake/help/latest/command/target_include_directories.html
function(vsf_system_include_directories)
  foreach(arg ${ARGV})
    if(IS_ABSOLUTE ${arg})
      set(path ${arg})
    else()
      set(path ${CMAKE_CURRENT_SOURCE_DIR}/${arg})
    endif()
    target_include_directories(vsf_interface SYSTEM INTERFACE ${path})
  endforeach()
endfunction()

# https://cmake.org/cmake/help/latest/command/target_compile_definitions.html
function(vsf_compile_definitions)
  target_compile_definitions(vsf_interface INTERFACE ${ARGV})
endfunction()

# https://cmake.org/cmake/help/latest/command/target_compile_options.html
function(vsf_compile_options)
  target_compile_options(vsf_interface INTERFACE ${ARGV})
endfunction()

# https://cmake.org/cmake/help/latest/command/target_link_libraries.html
function(vsf_link_libraries)
  target_link_libraries(vsf_interface INTERFACE ${ARGV})
endfunction()


function(set_ifndef variable value)
  if(NOT ${variable})
    set(${variable} ${value} ${ARGN} PARENT_SCOPE)
  endif()
endfunction()

function(vsf_cc_option)
  foreach(arg ${ARGV})
    target_cc_option(vsf_interface INTERFACE ${arg})
  endforeach()
endfunction()

function(vsf_cc_option_fallback option1 option2)
    target_cc_option_fallback(vsf_interface INTERFACE ${option1} ${option2})
endfunction()

function(vsf_ld_options)
    target_ld_options(vsf_interface INTERFACE ${ARGV})
endfunction()

macro(vsf_library_named name)
  set(VSF_CURRENT_LIBRARY ${name})
  add_library(${name} STATIC "")
  vsf_append_cmake_library(${name})

  target_link_libraries(${name} PUBLIC vsf_interface)
endmacro()

macro(assert_exists var)
  if(NOT EXISTS ${${var}})
    message(FATAL_ERROR "No such file or directory: ${var}: '${${var}}'")
  endif()
endmacro()
#[[
extensions end
]]

set(CMAKE_C_COMPILER_FORCED   1)
set(CMAKE_CXX_COMPILER_FORCED 1)


#[[
toolchain
]]
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

if(DEFINED CONFIG_TOOLCHAIN_VARIANT)
  set(VSF_TOOLCHAIN_VARIANT ${CONFIG_TOOLCHAIN_VARIANT})
else()
  set(VSF_TOOLCHAIN_VARIANT gnuarmemb)
endif()

if(VSF_TOOLCHAIN_VARIANT STREQUAL "gnuarmemb")
  if(DEFINED ENV{GNUARMEMB_TOOLCHAIN_PATH})
    set(GNUARMEMB_TOOLCHAIN_PATH $ENV{GNUARMEMB_TOOLCHAIN_PATH})
  else()
    if(WIN32)
      set(GNUARMEMB_TOOLCHAIN_PATH "%ProgramFiles(x86)%/GNU Tools Arm Embedded/7 2018-q2-update")
    elseif(UNIX)
      #TODO
    elseif(APPLE)
      #TODO
    endif()
  endif()

  assert_exists(GNUARMEMB_TOOLCHAIN_PATH)
  message(STATUS ${GNUARMEMB_TOOLCHAIN_PATH})

  set(TOOLCHAIN_HOME ${GNUARMEMB_TOOLCHAIN_PATH})
  set(COMPILER gcc)
  set(CROSS_COMPILE_TARGET arm-none-eabi)
  set(SYSROOT_TARGET       arm-none-eabi)
  set(CROSS_COMPILE ${TOOLCHAIN_HOME}/bin/${CROSS_COMPILE_TARGET}-)
  set(SYSROOT_DIR   ${TOOLCHAIN_HOME}/${SYSROOT_TARGET})
elseif(VSF_TOOLCHAIN_VARIANT STREQUAL "iarcc")
  message(FATAL_ERROR "iarcc not supported")
elseif(VSF_TOOLCHAIN_VARIANT STREQUAL "armcc")
  message(FATAL_ERROR "armcc not supported")
endif()

if(COMPILER STREQUAL "gcc")
  set_ifndef(CC gcc)
  set_ifndef(C++ g++)

  find_program(CMAKE_C_COMPILER ${CROSS_COMPILE}${CC}   PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_OBJCOPY    ${CROSS_COMPILE}objcopy PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_OBJDUMP    ${CROSS_COMPILE}objdump PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_AS         ${CROSS_COMPILE}as      PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_LINKER     ${CROSS_COMPILE}ld      PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_AR         ${CROSS_COMPILE}ar      PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_RANLIB     ${CROSS_COMPILE}ranlib  PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_READELF    ${CROSS_COMPILE}readelf PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_GDB        ${CROSS_COMPILE}gdb     PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)
  find_program(CMAKE_NM         ${CROSS_COMPILE}nm      PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)

  if(CONFIG_CPLUSPLUS)
    set(cplusplus_compiler ${CROSS_COMPILE}${C++})
  else()
    if(EXISTS ${CROSS_COMPILE}${C++})
      set(cplusplus_compiler ${CROSS_COMPILE}${C++})
    else()
      set(cplusplus_compiler ${CMAKE_C_COMPILER})
    endif()
  endif()
  find_program(CMAKE_CXX_COMPILER ${cplusplus_compiler} PATH ${TOOLCHAIN_HOME} NO_DEFAULT_PATH)

  message(STATUS "1/11  C_COMPILER          : ${CMAKE_C_COMPILER}")
  message(STATUS "2/11  CMAKE_OBJCOPY       : ${CMAKE_OBJCOPY}")
  message(STATUS "3/11  CMAKE_OBJDUMP       : ${CMAKE_OBJDUMP}")
  message(STATUS "4/11  CMAKE_AS            : ${CMAKE_AS}")
  message(STATUS "5/11  CMAKE_LINKER        : ${CMAKE_LINKER}")
  message(STATUS "6/11  CMAKE_AR            : ${CMAKE_AR}")
  message(STATUS "7/11  CMAKE_RANLIB        : ${CMAKE_RANLIB}")
  message(STATUS "8/11  CMAKE_READELF       : ${CMAKE_READELF}")
  message(STATUS "9/11  CMAKE_GDB           : ${CMAKE_GDB}")
  message(STATUS "10/11 CMAKE_NM            : ${CMAKE_NM}")
  message(STATUS "11/11 CMAKE_CXX_COMPILER  : ${CMAKE_CXX_COMPILER}")

  set(NOSTDINC "")

  if("${VSFHAL_ARCH}" STREQUAL "arm")
    if(DEFINED VSFHAL_CPU_SERIES)
      #[[ VSFHAL_CPU_SERIES lists:
      cortex-m0
      cortex-m0plus
      cortex-m3
      cortex-m4
      cortex-m7
      cortex-m23
      cortex-m33
      cortex-m33+nodsp
      ]]
      set(GCC_M_CPU ${VSFHAL_CPU_SERIES})
    else()
      message(FATAL_ERROR "Expected VSFHAL_CPU_SERIES to be defined")
    endif()
  else()
    message(FATAL_ERROR "Expected VSFHAL_ARCH to be defined")
  endif()

  if("${VSFHAL_ARCH}" STREQUAL "arm")
    list(APPEND TOOLCHAIN_C_FLAGS
      -mthumb
      -mcpu=${GCC_M_CPU}
    )

    set(FPU_FOR_cortex-m4      fpv4-sp-d16)
    set(FPU_FOR_cortex-m7      fpv5-d16)
    set(FPU_FOR_cortex-m33     fpv5-sp-d16)

    if(CONFIG_FLOAT)
      list(APPEND TOOLCHAIN_C_FLAGS -mfpu=${FPU_FOR_${GCC_M_CPU}})
      if(CONFIG_FP_SOFTABI)
        list(APPEND TOOLCHAIN_C_FLAGS -mfloat-abi=softfp)
      elseif(CONFIG_FP_HARDABI)
        list(APPEND TOOLCHAIN_C_FLAGS -mfloat-abi=hard)
      endif()
    endif()

    execute_process(
      COMMAND ${CMAKE_C_COMPILER} ${TOOLCHAIN_C_FLAGS} --print-libgcc-file-name
      OUTPUT_VARIABLE LIBGCC_FILE_NAME
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    assert_exists(LIBGCC_FILE_NAME)

    get_filename_component(LIBGCC_DIR ${LIBGCC_FILE_NAME} DIRECTORY)

    assert_exists(LIBGCC_DIR)

    LIST(APPEND LIB_INCLUDE_DIR "-L\"${LIBGCC_DIR}\"")
    LIST(APPEND TOOLCHAIN_LIBS gcc)

    if(SYSROOT_DIR)
      execute_process(
        COMMAND ${CMAKE_C_COMPILER} ${TOOLCHAIN_C_FLAGS} --print-multi-directory
        OUTPUT_VARIABLE NEWLIB_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      set(LIBC_LIBRARY_DIR "\"${SYSROOT_DIR}\"/lib/${NEWLIB_DIR}")
      set(LIBC_INCLUDE_DIR ${SYSROOT_DIR}/include)
    endif()

    foreach(isystem_include_dir ${NOSTDINC})
      list(APPEND isystem_include_flags -isystem "\"${isystem_include_dir}\"")
    endforeach()
    set(CMAKE_REQUIRED_FLAGS -nostartfiles -nostdlib ${isystem_include_flags} -Wl,--unresolved-symbols=ignore-in-object-files)
    string(REPLACE ";" " " CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
  else()
      message(FATAL_ERROR "Expected VSFHAL_ARCH to be defined")
  endif()
endif()
#[[
toolchain end
]]


if(NOT DEFINED CONFIG_APP_BIN_NAME)
  set(CONFIG_APP_BIN_NAME vsf)
  set(APP_NAME ${CONFIG_APP_BIN_NAME})
endif()

set(APP_ELF_NAME   ${APP_NAME}.elf)
set(APP_BIN_NAME   ${APP_NAME}.bin)
set(APP_HEX_NAME   ${APP_NAME}.hex)
set(APP_MAP_NAME   ${APP_NAME}.map)
set(APP_LST_NAME   ${APP_NAME}.lst)
set(APP_S19_NAME   ${APP_NAME}.s19)
set(APP_EXE_NAME   ${APP_NAME}.exe)
set(APP_STAT_NAME  ${APP_NAME}.stat)
set(APP_STRIP_NAME ${APP_NAME}.strip)

vsf_library_named(app)

add_subdirectory(${VSF_BASE} ${VSF_BINARY_DIR})

get_property(VSF_INTERFACE_LIBS_PROPERTY GLOBAL PROPERTY VSF_INTERFACE_LIBS)
foreach(vsf_lib ${VSF_INTERFACE_LIBS_PROPERTY})
  string(TOUPPER ${vsf_lib} vsf_lib_upper_case)
  target_link_libraries_ifdef(
    CONFIG_APP_LINK_WITH_${vsf_lib_upper_case}
    app
    PUBLIC
    ${vsf_lib}
    )
endforeach()
