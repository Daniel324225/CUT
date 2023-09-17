add_library(analyzer STATIC src/analyzer.c)
target_link_libraries(analyzer PRIVATE compiler_options lock watchdog logger reader)
target_include_directories(analyzer PUBLIC inc)