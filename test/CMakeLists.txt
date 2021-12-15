include(dtest)

# ------------------------------ Range and Slice -------------------------------

add_dtests(NAME test_range FILES test_range.cpp LIBS parlay)
#add_dtests(NAME test_allocator FILES test_allocator.cpp LIBS parlay)
