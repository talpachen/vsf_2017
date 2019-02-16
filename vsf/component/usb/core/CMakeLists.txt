vsf_include_directories(.)

vsf_sources_ifdef(
    VSFCOMPO_USBD
    vsfusbd.c
)

vsf_sources_ifdef(
    VSFCOMPO_USBH
    vsfusbh.c
)

add_subdirectory_ifdef(VSFCOMPO_DWC2 class)