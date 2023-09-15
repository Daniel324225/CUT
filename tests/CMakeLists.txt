add_executable(lock_test lock.c)
target_link_libraries(lock_test PRIVATE lock)