add_library(printer STATIC src/printer.c)
target_link_libraries(printer PRIVATE compiler_options lock watchdog logger analyzer)
target_include_directories(printer PUBLIC inc)