add_library(logger STATIC src/logger.c src/log_queue.c)
target_link_libraries(logger PRIVATE compiler_options watchdog)
target_include_directories(logger PUBLIC inc)
