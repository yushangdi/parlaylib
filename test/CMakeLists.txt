include(CTest)
include(GoogleTest)

# ------------------------------ Range and Slice -------------------------------

#add_dtests(NAME test_allocator FILES test_allocator.cpp LIBS parlay)

add_executable(test_allocator test_allocator.cpp)
target_link_libraries(test_allocator PRIVATE gtest_main parlay)
target_compile_definitions(test_allocator PRIVATE DEBUG)

gtest_discover_tests(test_allocator DISCOVERY_MODE PRE_TEST)
