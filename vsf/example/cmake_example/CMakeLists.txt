add_subdirectory(config)

vsf_include_directories(.)

target_sources(app PRIVATE usrapp.c)