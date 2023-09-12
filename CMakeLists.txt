cmake_minimum_required(VERSION 3.20)
project("CUT")

add_library(compiler_options INTERFACE)
target_compile_features(compiler_options INTERFACE c_std_11)
target_compile_options(compiler_options INTERFACE
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra>
  $<$<C_COMPILER_ID:Clang>:-Weverything>
  -Wconversion -pedantic -Werror
)

add_executable(CUT src/main.c)
target_link_libraries(CUT PRIVATE compiler_options)
