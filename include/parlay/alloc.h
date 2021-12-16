#ifndef PARLAY_ALLOC_H
#define PARLAY_ALLOC_H

#include "internal/block_allocator.h"

namespace parlay {



template <typename T>
class type_allocator {
public:
  static constexpr inline size_t default_alloc_size = 0;
  static constexpr inline bool initialized = true;
  static inline block_allocator allocator = block_allocator(sizeof(T));

  static T* alloc() { return static_cast<T*>(allocator.alloc()); }
  static void free(T* ptr) { allocator.free(static_cast<void*>(ptr)); }

  // for backward compatibility
  static void init(size_t, size_t) {};
  static void init() {};
  static void reserve(size_t n = default_alloc_size) { allocator.reserve(n); }
  static void finish() { allocator.clear(); }
  static size_t block_size () { return allocator.block_size(); }
  static size_t num_allocated_blocks() { return allocator.num_allocated_blocks(); }
  static size_t num_used_blocks() { return allocator.num_used_blocks(); }
  static size_t num_used_bytes() { return num_used_blocks() * block_size(); }
  static void print_stats() { allocator.print_stats(); }
};


}  // namespace parlay

#endif  // PARLAY_ALLOC_H
