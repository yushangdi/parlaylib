#include "get_time.h"

#include "sequence.h"
#include "random.h"
#include "primitives.h"
#include "monoid.h"
#include "internal/counting_sort.h"
#include "internal/collect_reduce.h"
#include "internal/integer_sort.h"
#include "internal/sample_sort.h"
#include "internal/merge.h"
#include "internal/merge_sort.h"
//#include "bag.h"
//#include "hash_table.h"
//#include "sparse_mat_vec_mult.h"
//#include "stlalgs.h"
//#include "random_shuffle.h"
//#include "range_min.h"

#include <iostream>
#include <ctype.h>
#include <math.h>
#include <assert.h>

static timer bt;
using uchar = unsigned char;

#define time(_var,_body)    \
  bt.start();               \
  _body;		    \
  double _var = bt.stop();

template<typename T>
double t_tabulate(size_t n, bool) {
  auto f = [] (size_t i) -> T {return i;};
  time(t, parlay::tabulate(n, f););
  return t;
}

template<typename T>
double t_map(size_t n, bool) {
  parlay::sequence<T> In(n, (T) 1);
  auto f = [&] (size_t i) -> T {return In[i];};
  time(t, parlay::tabulate(n, f));
  return t;
}

template<typename T>
double t_reduce_add(size_t n, bool) {
  parlay::sequence<T> S(n, (T) 1);
  time(t, parlay::reduce(S));
  return t;
}

// double t_map_reduce_128(size_t n, bool) {
//   int stride = 16;
//   parlay::sequence<size_t> S(n*stride, (size_t) 1);
//   auto get = [&] (size_t i) {
//     // gives marginal improvement (5% or so on aware)
//     __builtin_prefetch (&S[(i+4)*stride], 0, 3);
//     return S[i*stride];};
//   auto Sa = parlay::delayed_sequence<size_t>(n, get);
//   time(t, parlay::reduce(Sa.slice(0,n-4)););
//   return t;
// }

template<typename T>
double t_scan_add(size_t n, bool) {
  parlay::sequence<T> In(n, (T) 1);
  parlay::sequence<T> Out;
  T sum;
  time(t, std::tie(Out,sum) = parlay::scan(In););
  return t;
}

template<typename T>
double t_pack(size_t n, bool) {
  auto flags = parlay::tabulate(n, [] (size_t i) -> bool {return i%2;});
  auto In = parlay::tabulate(n, [] (size_t i) -> T {return i;});
  time(t, parlay::pack(In, flags););
  return t;
}

// template<typename T>
// double t_split3_old(size_t n, bool) {
//   parlay::sequence<uchar> flags(n, [] (size_t i) -> uchar {return i%3;});
//   parlay::sequence<T> In(n, [] (size_t i) -> T {return i;});
//   parlay::sequence<T> Out(n, (T) 0);
//   time(t, parlay::split_three(In, Out, flags););
//   return t;
// }

// template<typename T>
// double t_split3(size_t n, bool) {
//   parlay::random r(0);
//   parlay::sequence<T> In(n, [&] (size_t i) {return r.ith_rand(i);});
//   parlay::sequence<T> Out(n, (T) 0);
//   time(t, parlay::p_split3(In, Out.slice(), std::less<T>()););
//   return t;
// }

// template<typename T>
// double t_partition(size_t n, bool) {
//   parlay::random r(0);
//   parlay::sequence<T> In(n, [&] (size_t i) -> size_t {return r.ith_rand(i)%n;});
//   parlay::sequence<T> Out;
//   auto f = parlay::delayed_seq<bool>(n, [&] (size_t i) {return In[i] < n/((size_t) 2);});
//   size_t m;
//   time(t, std::tie(Out,m) = parlay::split_two(In, f););
//   return t;
// }

// #if defined(CILK)
// #include "reducer.h"
// double t_histogram_reducer(size_t n, bool) {
//   parlay::random r(0);
//   constexpr int count = 1024;
//   histogram_reducer<int,count> red;
//   using aa = std::array<size_t,8>;
//   parlay::sequence<aa> In(n, [&] (size_t i) {aa x; x[0] = r.ith_rand(i) % count; return x;});
//   auto f = [&] (size_t i) { red->add_value(In[i][0]);};
//   time(t, parallel_for(0, n, f););
//   //cout << red.get_value()[0] << endl;
//   return t;
// }
// #else
// double t_histogram_reducer(size_t, bool) {
//   return 1.0;
// }
// #endif

template<typename T>
double t_gather(size_t n, bool) {
  parlay::random r(0);
  auto in = parlay::tabulate(n, [&] (size_t i) -> T {return i;});
  auto idx = parlay::tabulate(n, [&] (size_t i) -> T {return r.ith_rand(i)%n;});
  auto f = [&] (size_t i) -> T {
    // prefetching helps significantly
    __builtin_prefetch (&in[idx[i+4]], 0, 1);
    return in[idx[i]];};
  // note problem with prefetching since will go over end for last 4 iterations
  if (n < 4) return 0.0;
  time(t, parlay::tabulate(n-4, f););  
  return t;
}

template<typename T>
double t_scatter(size_t n, bool) {
  parlay::random r(0);
  parlay::sequence<T> out(n, (T) 0);
  auto idx = parlay::tabulate(n, [&] (size_t i) -> T {return r.ith_rand(i)%n;});
  auto f = [&] (size_t i) {
    // prefetching makes little if any difference
    //__builtin_prefetch (&out[idx[i+4]], 1, 1);
      out[idx[i]] = i;};
  if (n < 4) return 0.0;
  time(t, parlay::parallel_for(0, n-4, f););
  return t;
}

template<typename T>
double t_write_add(size_t n, bool) {
  parlay::random r(0);
  parlay::sequence<T> out(n, (T) 0);
  //parlay::sequence<std::atomic<T>> out(n);
  //parlay::parallel_for(0,n,[&] (size_t i) {std::atomic_init(&out[i], (T) 0);});
  auto idx = parlay::tabulate(n, [&] (size_t i) -> T {return r.ith_rand(i)%n;});
  auto f = [&] (size_t i) {
    // putting write prefetch in slows it down
    //__builtin_prefetch (&out[idx[i+4]], 0, 1);
    //__sync_fetch_and_add(&out[idx[i]],1);};
    parlay::write_add(&out[idx[i]],1);};
  if (n < 4) return 0.0;
  time(t, parlay::parallel_for(0, n-4, f););
  return t;
}

template<typename T>
double t_write_min(size_t n, bool) {
  parlay::random r(0);
  //parlay::sequence<std::atomic<T>> out(n);
  //parlay::parallel_for(0,n,[&] (size_t i) {std::atomic_init(&out[i], (T) n);});
  parlay::sequence<T> out(n, (T) n);
  auto idx = parlay::tabulate(n, [&] (size_t i) -> T {return r.ith_rand(i)%n;});
  auto f = [&] (size_t i) {
    // putting write prefetch in slows it down
    //__builtin_prefetch (&out[idx[i+4]], 1, 1);
    parlay::write_min(&out[idx[i]], (T) i, std::less<T>());};
  if (n < 4) return 0.0;
  time(t, parlay::parallel_for(0, n-4, f););
  return t;
}

template<typename T>
double t_shuffle(size_t n, bool) {
  auto in = parlay::tabulate(n, [&] (size_t i) -> T {return i;});
  time(t, parlay::random_shuffle(in, n););
  return t;
}

template<typename T>
bool check_histogram(parlay::sequence<T> const &in, parlay::sequence<T> const &out) {
  // size_t m = out.size();
  // auto a = sort(in, std::less<T>());
  // auto b = get_counts(a, [&] (T a) {return a;}, m);
  // size_t err_loc = parlay::find_if_index(m, [&] (size_t i) {return out[i] != b[i];});
  // if (err_loc != m) {
  //   cout << "ERROR in histogram at location "
  // 	 << err_loc << ", got " << out[err_loc] << ", expected " << b[err_loc] << endl;
  //   return false;
  // }
  return true;
}

template<typename T>
double t_histogram(size_t n, bool check) {
  parlay::random r(0);
  auto in = parlay::tabulate(n, [&] (size_t i) -> T {return r.ith_rand(i)%n;});
  parlay::sequence<T> out;
  //auto get_key = [&] (T a) {return a;};
  //auto get_val = [&] (T a) {return (T) 1;};
  //time(t, out = parlay::collect_reduce(in, get_key, get_val, parlay::addm<T>(), n););
  time(t, out = parlay::histogram(in, (T) n););
  if (check) check_histogram(in, out);
  return t;
}

template<typename T>
double t_histogram_few(size_t n, bool check) {
  parlay::random r(0);
  auto in = parlay::tabulate(n, [&] (size_t i) -> T {return r.ith_rand(i)%256;});
  parlay::sequence<T> out;
  //auto get_key = [&] (T a) {return a;};
  //auto get_val = [&] (T a) {return (T) 1;};
  //time(t, out = parlay::collect_reduce(in, get_key, get_val, parlay::addm<T>(), 256););
  time(t, out = parlay::histogram(in, (T) 256););
  if (check) check_histogram(in, out);
  return t;
}

template<typename T>
double t_histogram_same(size_t n, bool check) {
  parlay::sequence<T> in(n, (T) 10311);
  parlay::sequence<T> out;
  //auto get_key = [&] (T a) {return a;};
  //auto get_val = [&] (T a) {return (T) 1;};
  //time(t, out = parlay::collect_reduce(in, get_key, get_val, parlay::addm<T>(), n););
  time(t, out = parlay::histogram(in, (T) n););
  if (check) check_histogram(in, out);
  return t;
}

// this checks against a given sort
template<typename T, typename Cmp>
bool check_sort(parlay::sequence<T> const &in, parlay::sequence<T> const &out,
		Cmp less, std::string sort_name) {
  // size_t n = in.size();
  // auto a = parlay::merge_sort(in, std::less<T>());
  // size_t err_loc = parlay::find_if_index(n, [&] (size_t i) {
  //     return less(a[i],out[i]) || less(out[i],a[i]);});
  // if (err_loc != n) {
  //   cout << "ERROR in " << sort_name << " at location " << err_loc << endl;
  //   return false;
  // }
  return true;
}

template<typename T>
double t_sort(size_t n, bool check) {
  parlay::random r(0);
  auto in = parlay::tabulate(n, [&] (size_t i) -> T {return r.ith_rand(i)%n;});
  parlay::sequence<T> out;
  time(t, out = parlay::internal::sample_sort(parlay::make_slice(in), std::less<T>()););
  if (check) check_sort(in, out, std::less<T>(), "sample sort");
  return t;
}

// // no check since it is used for the sort for checking, and hence
// // checked against the other sorts
// template<typename T>
// double t_merge_sort(size_t n, bool) {
//   parlay::random r(0);
//   parlay::sequence<T> in(n, [&] (size_t i) {return r.ith_rand(i)%n;});
//   parlay::sequence<T> out;
//   time(t, parlay::merge_sort_inplace(in.slice(), std::less<T>()););
//   return t;
// }

// template<typename T>
// double t_quicksort(size_t n, bool check) {
//   parlay::random r(0);
//   parlay::sequence<T> in(n, [&] (size_t i) {return r.ith_rand(i)%n;});
//   parlay::sequence<T> copy;
//   if (check) copy = in;
//   time(t, parlay::p_quicksort_inplace(in.slice(), std::less<T>()););
//   if (check) check_sort(copy, in, std::less<T>(), "quicksort");
//   return t;
// }

template<typename T>
double t_count_sort_bits(size_t n, size_t bits) {
  parlay::random r(0);
  size_t num_buckets = (1<<bits);
  size_t mask = num_buckets - 1;
  auto in = parlay::tabulate(n, [&] (size_t i) -> T {return r.ith_rand(i);});
  parlay::sequence<T> out(n);
  auto f = [&] (size_t i) {return in[i] & mask;};
  auto keys = parlay::delayed_sequence<unsigned char>(n, f);
  time(t, parlay::internal::count_sort(parlay::make_slice(in), parlay::make_slice(out),
				       parlay::make_slice(keys.begin(),keys.end()), num_buckets););
  for (size_t i=1; i < n; i++) {
    if ((out[i-1] & mask) > (out[i] & mask)) {
      std::cout << "ERROR in count sort at: " << i << std::endl;
      abort();
    }
  }
  return t;
}

template<typename T>
double t_count_sort_8(size_t n, bool) {return t_count_sort_bits<T>(n, 8);}

// template<typename T>
// double t_count_sort_2(size_t n, bool) {return t_count_sort_bits<T>(n, 2);}

// template<typename T>
// double t_collect_reduce_pair_dense(size_t n, bool check) {
//   using par = std::pair<T,T>;
//   parlay::random r(0);
//   parlay::sequence<par> S(n, [&] (size_t i) -> par {
//       return par(r.ith_rand(i) % n, 1);});
//   parlay::sequence<T> out;
//   auto get_key = [&] (par a) {return a.first;};
//   auto get_val = [&] (par a) {return a.second;};
//   time(t, out = parlay::collect_reduce(S, get_key, get_val, parlay::addm<T>(), n););
//   if (check)
//     check_histogram(parlay::sequence<T>(n, [&] (size_t i) {return S[i].first;}),
// 		    out);
//   return t;
// }

// template<typename T>
// double t_collect_reduce_pair_sparse(size_t n, bool) {
//   using par = std::pair<T,T>;
//   parlay::random r(0);
//   struct hasheq {
//     static inline size_t hash(par a) {return parlay::hash64_2(a.first);}
//     static inline bool eql(par a, par b) {return a.first == b.first;}
//   };
//   parlay::sequence<par> S(n, [&] (size_t i) -> par {
//       return par(r.ith_rand(i) % n, 1);});
//   time(t, parlay::collect_reduce_sparse(S, hasheq(), parlay::addm<T>()););
//   return t;
// }

// template<typename T>
// double t_collect_reduce_8(size_t n, bool) {
//   using par = std::pair<T,T>;
//   parlay::random r(0);
//   size_t num_buckets = (1<<8);
//   parlay::sequence<par> S(n, [&] (size_t i) {
//       return par(r.ith_rand(i) % num_buckets, 1);});
//   auto get_key = [&] (par a) {return a.first;};
//   auto get_val = [&] (par a) {return a.first;};
//   time(t, parlay::collect_reduce(S, get_key, get_val, parlay::addm<T>(), num_buckets););
//   return t;
// }

// // template<typename T>
// // double t_collect_reduce_8_tuple(size_t n, bool check) {
// //   parlay::random r(0);
// //   size_t num_buckets = (1<<8);
// //   size_t mask = num_buckets - 1;
// //   using sums = std::tuple<float,float,float,float>;

// //   auto bucket = [&] (size_t i) -> uchar { return r.ith_rand(i) & mask; };
// //   auto keys = parlay::delayed_seq<unsigned char>(n, bucket);

// //   auto sum = [] (sums a, sums b) -> sums {
// //     return sums(std::get<0>(a)+std::get<0>(b), std::get<1>(a)+std::get<1>(b),
// // 		std::get<2>(a)+std::get<2>(b), std::get<3>(a)+std::get<3>(b));
// //   };

// //   parlay::sequence<sums> in(n, [&] (size_t i) -> sums {
// //       return sums(1.0,1.0,1.0,1.0);});

// //   auto monoid = make_monoid(sum, sums(0.0,0.0,0.0,0.0));

// //   time(t,
// //        parlay::collect_reduce<sums>(in, keys, num_buckets, monoid););
// //   return t;
// // }


template<typename T>
double t_integer_sort_pair(size_t n, bool check) {
  using par = std::pair<T,T>;
  parlay::random r(0);
  size_t bits = sizeof(T)*8;
  auto S = parlay::tabulate(n, [&] (size_t i) -> par {
				 return par(r.ith_rand(i),i);});
  parlay::sequence<par> R;
  auto first = [] (par a) {return a.first;};
  time(t, R = parlay::internal::integer_sort(parlay::make_slice(S), first, bits););

  auto less = [] (par a, par b) {return a.first < b.first;};
  if (check) check_sort(S, R, less, "integer sort pair");
  return t;
}

template<typename T>
double t_integer_sort(size_t n, bool check) {
  parlay::random r(0);
  size_t bits = sizeof(T)*8;
  auto S = parlay::tabulate(n, [&] (size_t i) -> T {
				 return r.ith_rand(i);});
  auto identity = [] (T a) {return a;};
  parlay::sequence<T> R;
  //time(t, parlay::integer_sort_inplace(S.slice(),identity,bits););
  time(t, R = parlay::internal::integer_sort(parlay::make_slice(S), identity, bits););
  if (check) check_sort(S, R, std::less<T>(), "integer sort");
  return t;
}

typedef unsigned __int128 long_int;
double t_integer_sort_128(size_t n, bool) {
  parlay::random r(0);
  size_t bits = parlay::log2_up(n);
  auto S = parlay::tabulate(n, [&] (size_t i) -> long_int {
      return r.ith_rand(2*i) + (((long_int) r.ith_rand(2*i+1)) << 64) ;});
  auto identity = [] (long_int a) {return a;};
  parlay::sequence<long_int> out;
  time(t, out = parlay::internal::integer_sort(parlay::make_slice(S),identity,bits););
  return t;
}

// template<typename T>
// double t_merge(size_t n, bool) {
//   parlay::sequence<T> in1(n/2, [&] (size_t i) {return 2*i;});
//   parlay::sequence<T> in2(n-n/2, [&] (size_t i) {return 2*i+1;});
//   parlay::sequence<T> out;
//   time(t, out = parlay::merge(in1, in2, std::less<T>()););
//   return t;
// }

// template<typename T>
// double t_remove_duplicates(size_t n, bool) {
//   parlay::random r(0);
//   parlay::sequence<T> In(n, [&] (size_t i) -> T {return r.ith_rand(i) % n;});
//   time(t, parlay::remove_duplicates(In););
//   return t;
// }

// template <typename T, typename F>
// static T my_reduce(parlay::sequence<T> const &s, size_t start, size_t end, F f) {
//   if (end - start == 1) return s[start];
//   size_t h = (end + start)/2;
//   T r, l;
//   auto left = [&] () {r = my_reduce(s, h, end, f);};
//   auto right = [&] () {l = my_reduce(s, start, h, f);};
//   par_do_if(h > 100, left, right);
//   return f(l,r);
// }

// template<typename T>
// double t_bag(size_t n, bool) {
//   using TB = parlay::bag<T>;
//   TB::init();
//   parlay::sequence<TB> In(n, [&] (size_t i) -> TB {return TB((T) i);});
//   time(t, TB x = my_reduce(In, 0, n, TB::append); x.flatten(););
//   return t;
// }

// template<typename s_size_t, typename T>
// double t_mat_vec_mult(size_t n, bool) {
//   parlay::random r(0);
//   size_t degree = 5;
//   size_t m = degree*n;
//   parlay::sequence<s_size_t> starts(n+1, [&] (size_t i) {
//       return degree*i;});
//   parlay::sequence<s_size_t> columns(m, [&] (size_t i) {
//       return r.ith_rand(i)%n;});
//   parlay::sequence<T> values(m, (T) 1);
//   parlay::sequence<T> in(n, (T) 1);
//   parlay::sequence<T> out(n, (T) 0);
//   auto add = [] (T a, T b) { return a + b;};
//   auto mult = [] (T a, T b) { return a * b;};

//   time(t, mat_vec_mult(starts, columns, values, in, out.slice(), mult, add););
//   return t;
// }

// template<typename T>
// double t_range_min(size_t n, bool) {
//   parlay::sequence<T> In(n, [&] (size_t) {return 5;});
//   In[n/2] = 0;
//   time(t,
//        auto foo = parlay::make_range_min(In, std::less<T>());
//        parallel_for(0, n-1, [&] (size_t i) {foo.query(0,i);}););
//   if (foo.query(0,n-1) != n/2 || foo.query(0,n/2-1) != 0 ||
//       foo.query(0,n/2) != n/2 || foo.query(n/2+1, n-1) != n/2+1) {
//     cout << "error in range min query " << endl;
//     abort();
//   }
//   return t;
// }

// template<typename T>
// double t_find_mid(size_t n, bool check) {
//   parlay::sequence<T> In(n, [&] (size_t) {return 0;});
//   In[n/2] = 1;
//   size_t idx;
//   time(t, idx = parlay::find(In, 1););
//   if (check)
//     if (idx != n/2)
//       cout << "error in find " << endl;
//   return t;
// }

// template<typename T>
// double t_lexicograhic_compare(size_t n, bool check) {
//   parlay::sequence<T> In1(n, [&] (size_t) {return 0;});
//   parlay::sequence<T> In2(n, [&] (size_t) {return 0;});
//   In1[n/2] = 1;
//   bool ls;
//   time(t, ls = parlay::lexicographical_compare(In1, In2, std::less<T>()););
//   if (check)
//     if (ls)
//       cout << "error in lexicographical_compare " << endl;
//   return t;
// }

