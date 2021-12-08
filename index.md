---
title: ParlayLib - A Toolkit for Programming Parallel Algorithms on Shared-Memory Multicore Machines
---

# ParlayLib

ParlayLib is a C++ library for developing efficient parallel algorithms and software on shared-memory multicore machines. It provides additional tools and primitives that go beyond what is available in the C++ standard library, and simplifies the task of programming provably efficient and scalable parallel algorithms. It consists of a sequence data type (analogous to std::vector), many parallel routines and algorithms, a work-stealing scheduler to support nested parallelism, and a scalable memory allocator. It has been developed over a period of seven years and used in a variety of software including the [PBBS benchmark suite](http://www.cs.cmu.edu/~pbbs/benchmarks.html), the [Ligra](http://jshun.github.io/ligra/), [Julienne](https://dl.acm.org/doi/pdf/10.1145/3087556.3087580), and [Aspen](https://github.com/ldhulipala/aspen) graph processing frameworks, the [Graph Based Benchmark Suite](https://github.com/ParAlg/gbbs), and the [PAM](https://cmuparlay.github.io/PAMWeb/) library for parallel balanced binary search trees, and an implementation of the TPC-H benchmark suite.

Parlay is designed to be reasonably portable by being built upon mostly standards-compliant modern C++. It builds on [GCC](https://gcc.gnu.org/) and [Clang](https://clang.llvm.org/) on Linux, GCC and Apple Clang on OSX, and Microsoft Visual C++ ([MSVC](https://visualstudio.microsoft.com/vs/)) and [MinGW](http://www.mingw.org/) on Windows. It is also tested on GCC and Clang via Windows Subsystem for Linux ([WSL](https://docs.microsoft.com/en-us/windows/wsl/about)) and [Cygwin](https://www.cygwin.com/).

## Getting started

See the [installation](./installation.md) guide for instructions on installing Parlay and including it in your program. Parlay is a header-only library with no external dependencies, so once you've got it, its really easy to use. The most important components of Parlay to become familiarized with are the **parallel for loop**, the **sequence** container, and Parlay's library of parallel algorithms. As an example, here is an implementation of a [prime number seive](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) using Parlay.

```c++
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>

parlay::sequence<long> prime_sieve(long n) {
  if (n < 2) return parlay::sequence<long>();
  else {
    long sqrt = std::sqrt(n);
    auto primes_sqrt = prime_sieve(sqrt);
    parlay::sequence<bool> flags(n+1, true);  // flags to mark the primes
    flags[0] = flags[1] = false;              // 0 and 1 are not prime
    parlay::parallel_for(0, primes_sqrt.size(), [&] (size_t i) {
      long prime = primes_sqrt[i];
      parlay::parallel_for(2, n/prime + 1, [&] (size_t j) {
        flags[prime * j] = false;
      }, 1000);
    }, 1);
    return parlay::pack_index<long>(flags);    // indices of the primes
  }
}
```

This code demonstrates several of the key features mentioned above. The parallel for loop `parlay::parallel_for(i,j,f)` iterates over the range `[i,j)` and invokes the function `f` at each index. The sequence container holds an array of bools that is suitable for parallel computation (it is initialized and destructed in parallel). The `pack_index` algorithm takes a range of boolean elements and returns a sequence consisting of the indices that contain true.
