add_library(lock STATIC src/lock.c)
target_link_libraries(lock PRIVATE compiler_options utility)
target_include_directories(lock PUBLIC inc)