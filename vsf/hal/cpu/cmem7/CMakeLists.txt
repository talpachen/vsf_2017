add_subdirectory(startup)
add_subdirectory_ifdef(VSFHAL_USART     usart)
add_subdirectory_ifdef(VSFHAL_USB       usb)

vsf_library()

vsf_include_directories(.)

vsf_library_sources(
    core.c
)
