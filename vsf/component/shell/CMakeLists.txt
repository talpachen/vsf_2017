vsf_include_directories(.)

vsf_sources_ifdef(
    VSFCOMPO_SHELL
    vsfshell.c
)