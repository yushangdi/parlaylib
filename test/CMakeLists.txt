include(dtest)

# ------------------------------ Range and Slice -------------------------------

add_dtests(NAME test_allocator FILES test_allocator.cpp LIBS parlay)
