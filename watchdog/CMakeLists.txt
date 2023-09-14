add_library(watchdog STATIC src/watchdog.c)
target_link_libraries(watchdog PRIVATE compiler_options)
target_include_directories(watchdog PUBLIC inc)
