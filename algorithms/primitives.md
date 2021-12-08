# Core Algorithms

<small>**Usage: `#include <parlay/primitives.h>`**</small>

Parlay's core algorithms provide parallel versions of the majority of the C++ standard library, and more.

## Reference

- [Tabulate](#tabulate)
- [Map](#map)
- [Copy](#copy)
- [Reduce](#reduce)
- [Scan](#scan)
- [Pack](#pack)
- [Filter](#filter)
- [Merge](#merge)
- [[Experimental] Histogram](#-experimental--histogram)
- [Sort](#sort)
- [Integer Sort](#integer-sort)
- [For each](#for-each)
- [Count](#count)
- [All of, any of, none of](#all-of--any-of--none-of)
- [Find](#find)
- [Adjacent find](#adjacent-find)
- [Mismatch](#mismatch)
- [Search](#search)
- [Find end](#find-end)
- [Equal](#equal)
- [Lexicographical compare](#lexicographical-compare)
- [Unique](#unique)
- [Min and max element](#min-and-max-element)
- [Reverse](#reverse)
- [Rotate](#rotate)
- [Is sorted](#is-sorted)
- [Is partitioned](#is-partitioned)
- [Remove](#remove)
- [Iota](#iota)
- [Flatten](#flatten)
- [Tokens](#tokens)
- [Split](#split)


### Tabulate

```c++
template<typename UnaryOp>
auto tabulate(size_t n, UnaryOp&& f)
```

```c++
template<typename UnaryOp>
auto delayed_tabulate(size_t n, UnaryOp&& f)
```

**tabulate** takes an integer n and and a function f of integers, and produces a sequence consisting of f(0), f(1), ..., f(n-1). **delayed_tabulate** does the same but returns a delayed sequence instead.

### Map

```c++
template<parlay::Range R, typename UnaryOp>
auto map(R&& r, UnaryOp&& f)
```

```c++
template<parlay::Range R, typename UnaryOp>
auto delayed_map(R&& r, UnaryOp&& f)
```

**map** takes a range `r` and a function `f` from the value type of that range, and produces the sequence consisting of `f(r[0]), f(r[1]), f(r[2]), ...`.

**delayed_map** is the same as map, but the resulting sequence is a delayed sequence. Note that **delayed_map** forwards the range argument to the closure owned by the delayed sequence, so if `r` is an rvalue reference, it will be moved into and owned by the delayed sequence. If it is an lvalue reference, the delayed sequence will only keep a reference to `r`, so `r` must remain alive as long as the delayed sequence does.

### Copy

```c++
template<parlay::Range R_in, parlay::Range R_out>
void copy(const R_in& in, R_out& out)
```

**copy** takes a given range and copies its elements into another range

### Reduce

```c++
template<parlay::Range R>
auto reduce(const R& r)
```

```c++
template<parlay::Range R, typename Monoid>
auto reduce(const R& r, Monoid&& m)
```

**reduce** takes a range and returns the reduction with respect some associative binary operation (addition by default). The associative operation is specified by a monoid object which is an object that has a `.identity` field, and a binary operator `f`.

### Scan

```c++
template<parlay::Range R>
auto scan(const R& r)
```

```c++
template<parlay::Range R>
auto scan_inclusive(const R& r)
```

```c++
template<parlay::Range R>
auto scan_inplace(R&& r)
```

```c++
template<parlay::Range R>
auto scan_inclusive_inplace(R&& r)
```

```c++
template<parlay::Range R, typename Monoid>
auto scan(const R& r, Monoid&& m)
```

```c++
template<parlay::Range R, typename Monoid>
auto scan_inclusive(const R& r, Monoid&& m)
```

```c++
template<parlay::Range R, typename Monoid>
auto scan_inplace(R&& r, Monoid&& m)
```

```c++
template<parlay::Range R, typename Monoid>
auto scan_inclusive_inplace(R& r, Monoid&& m)
```

**scan** computes a scan (aka prefix sum) with respect to an associative binary operation (addition by default).  The associative operation is specified by a monoid object which is an object that has a `.identity` field, and a binary operator `f`. Scan returns a pair, consisting of the partial sums, and the total.

By default, scan considers prefix sums excluding the final element. There is also **scan_inclusive**, which is inclusive of the final element of each prefix. There are also inplace versions of each of these (**scan_inplace**, **scan_inclusive_inplace**), which write the sums into the input and return the total.

### Pack

```c++
template<parlay::Range R, parlay::Range BoolSeq>
auto pack(const R& r, const BoolSeq& b)
```

```c++
template<parlay::Range R_in, parlay::Range BoolSeq, parlay::Range R_out>
auto pack_into(const R_in& in, const BoolSeq& b, R_out& out)
```

```c++
template<parlay::Range BoolSeq>
auto pack_index(const BoolSeq& b) 
```

```c++
template<typename IndexType, parlay::Range BoolSeq>
auto pack_index(const BoolSeq& b) 
```

**pack** takes an input a range a boolean indicator sequence, and returns a new sequence consisting of the elements of the range such that the element in the corresponding position in the indicator sequence is true.

Similarly, **pack_into** does the same thing but writes the answer into an existing range. **pack_index** takes a range of elements that are convertible to bool, and returns a sequence of indices such that the elements at those positions convert to true.

### Filter

```c++
template<parlay::Range R, typename UnaryPred>
auto filter(const R& r, UnaryPred&& f) 
```

```c++
template<parlay::Range R_in, parlay::Range R_out, typename UnaryPred>
auto filter_into(const R_in& in, R_out& out, UnaryPred&& f)
```

**filter** takes a range and a unary operator, and returns a sequence consisting of the elements of the range for which the unary operator returns true. Alternatively, **filter_into** does the same thing but writes the output into the given range and returns the number of elements that were kept.

### Merge

```c++
template<parlay::Range R1, parlay::Range R2>
auto merge(const R1& r1, const R2& r2)
```

```c++
template<parlay::Range R1, parlay::Range R2, typename BinaryPred>
auto merge(const R1& r1, const R2& r2, BinaryPred pred)
```

**merge** returns a sequence consisting of the elements of `r1` and `r2` in sorted order, assuming
that `r1` and `r2` are already sorted. An optional binary predicate can be used to specify the comparison operation.

### [Experimental] Histogram


```c++
template<parlay::Range R>
auto histogram_by_key(R&& A)
```

```c++
template <typename sum_type = size_t, parlay::Range R, typename Hash, typename Equal>
auto histogram_by_key(R&& A, Hash hash, Equal equal)
```

```c++
template<typename Integer_, parlay::Range R>
auto histogram_by_index(const R& A, Integer_ m)
```

These functions are currently experimental and their interfaces may change soon.

**histogram_by_key** takes a range `A` and returns a sequence of key-value pairs, where the keys are the unique elements of `A`, and the values are the number of occurences of the corresponding key in `A`. Keys must be equality-comparable and hashable. The keys are not guaranteed to be in sorted order. Optionally, custom unary and binary operators can be supplied that specify how to hash and compare keys. By default, `std::hash` and `std::equal_to` are used. An optional template argument, `sum_type` allows the type of the counter values to be customized. 

**histogram_by_index** takes an integer-valued range `A` and a maximum value `m` and returns a sequence of length `m`, such that the `i`'th value of the sequence contains the number of occurences of `i` in `A`.
### Sort

```c++
template<parlay::Range R>
auto sort(const R& in)
```

```c++
template<parlay::Range R, typename Compare>
auto sort(const R& in, Compare&& comp)
```

```c++
template<parlay::Range R>
auto stable_sort(const R& in)
```

```c++
template<parlay::Range R, typename Compare>
auto stable_sort(const R& in, Compare&& comp)
```

```c++
template<parlay::Range R>
void sort_inplace(R&& in)
```

```c++
template<parlay::Range R, typename Compare>
void sort_inplace(R&& in, Compare&& comp)
```

```c++
template<parlay::Range R>
void stable_sort_inplace(R&& in)
```

```c++
template<parlay::Range R, typename Compare>
void stable_sort_inplace(R&& in, Compare&& comp)
```


**sort** takes a given range and outputs a sorted copy (unlike the standard library, sort is not inplace by default). **sort_inplace** can be used to sort a given range in place. **stable_sort** and **stable_sort_inplace** are the same but guarantee that equal elements maintain their original relative order. All of these functions can optionally take a custom comparator object, which is a binary operator that evaluates to true if the first of the given elements should compare less than the second.

### Integer Sort

```c++
template<parlay::Range R>
auto integer_sort(const R& in)
```

```c++
template<parlay::Range R, typename Key>
auto integer_sort(const R& in, Key&& key)
```

```c++
template<parlay::Range R>
void integer_sort_inplace(R&& in)
```

```c++
template<parlay::Range R, typename Key>
void integer_sort_inplace(R&& in, Key&& key)
```

```c++
template<parlay::Range R, typename Key>
auto stable_integer_sort(const R& in, Key&& key)
```

```c++
template<parlay::Range R, typename Key>
void stable_integer_sort_inplace(R&& in, Key&& key)
```

**integer_sort** works just like sort, except that it is specialized to sort integer keys, and is significantly faster than ordinary sort. It can be used to sort ranges of integers, or ranges of arbitrary types if a unary operator is provided that can produce an integer key for any given element. **stable_integer_sort** and **stable_integer_sort_inplace** are guaranteed to maintain the relative order between elements with equal keys.

### For each

```c++
template <parlay::Range R, typename UnaryFunction>
void for_each(R&& r , UnaryFunction f)
```

**for_each** applies the given unary function to every element of the given range. The range may be constant, in which case the unary function should not attempt to modify it, or it may be mutable, in which case the function is allowed to modify it.

### Count

```c++
template <parlay::Range R, class T>
size_t count(const R& r, T const &value)
```

```c++
template <parlay::Range R, typename UnaryPredicate>
size_t count_if(const R& r, UnaryPredicate p)
```

**count** returns the number of elements in the given range that compare equal to the given value. **count_if** returns the number of elements that satisfy the predicate p.

### All of, any of, none of

```c++
template <parlay::Range R, typename UnaryPredicate>
bool all_of(const R& r, UnaryPredicate p)
```

```c++
template <parlay::Range R, typename UnaryPredicate>
bool any_of(const R& r, UnaryPredicate p)
```

```c++
template <parlay::Range R, typename UnaryPredicate>
bool none_of(const R& r, UnaryPredicate p)
```

**all_of**, **any_of**, and **none_of** return true if the given predicate is true for all, any, or none of the elements in the given range respectively.

### Find

```c++
template <parlay::Range R, typename T>
auto find(R&& r, T const &value)
```

```c++
template <parlay::Range R, typename UnaryPredicate>
auto find_if(R&& r, UnaryPredicate p)
```

```c++
template <parlay::Range R, typename UnaryPredicate>
auto find_if_not(R&& r, UnaryPredicate p)
```

```c++
template <parlay::Range R1, parlay::Range R2, typename BinaryPredicate>
auto find_first_of(R1&& r1, const R2& r2, BinaryPredicate p)
```

**find** returns an iterator to the first element in the given range that compares equal to the given value, or the end iterator if no such element exists. **find_if** returns the first element in the given range that satisfies the given predicate, or the end iterator if no such element exists. **find_if_not** similarly returns the first element that does not satisfy the given predicate, or the end iterator.

**find_first_of** returns an iterator to the first element in the range `r1` that compares equal to any of the elements in `r2`, or the end iterator of `r1` if no such element exists.

### Adjacent find

```c++
template <parlay::Range R>
auto adjacent_find(R&& r)
```

```c++
template <parlay::Range R, typename BinaryPred>
auto adjacent_find(R&& r, BinaryPred p)
```

**adjacent_find** returns an iterator to the first element in the given range that compares equal to the next element on its right, Optionally, a binary predicate can be supplied to dictate how two elements should compare equal.

### Mismatch

```c++
template <parlay::Range R1, parlay::Range R2>
size_t mismatch(R1&& r1, R2&& r2)
```

```c++
template <parlay::Range R1, parlay::Range R2, typename BinaryPred>
auto mismatch(R1&& r1, R2&& r2, BinaryPred p)
```

**mismatch** returns a pair of iterators corresponding to the first occurrence in which an element of `r1` is not equal to the element of `r2` in the same position. If no such occurrence exists, returns a pair containing the end iterator of `r1` and an iterator pointing to the corresponding position in `r2`. Optionally, a binary predicate can be supplied to dictate how two elements should compare equal.

### Search

```c++
template <parlay::Range R1, parlay::Range R2>
size_t search(R1&& r1, const R2& r2)
```

```c++
template <parlay::Range R1, parlay::Range R2, typename BinaryPred>
auto search(R1&& r1, const R2& r2, BinaryPred pred)
```

**search** returns an iterator to the beginning of the first occurrence of the range `r2` in `r1`, or the end iterator of `r1` if no such occurrence exists. Optionally, a binary predicate can be given to specify how two elements should compare equal.

### Find end

```c++
template <parlay::Range R1, parlay::Range R2>
auto find_end(R1&& r1, const R2& r2)
```

```c++
template <parlay::Range R1, parlay::Range R2, typename BinaryPred>
auto find_end(R1&& r1, const R2& r2, BinaryPred p)
```

**find_end** returns an iterator to the beginning of the last occurrence of the range `r2` in `r1`, or the end iterator of `r1` if no such occurrence exists. Optionally, a binary predicate can be given to specify how two elements should compare equal.

### Equal

```c++
template <parlay::Range R1, parlay::Range R2>
bool equal(const R1& r1, const R2& r2)
```

```c++
template <parlay::Range R1, parlay::Range R2, class BinaryPred>
bool equal(const R1& r1, const R2& r2, BinaryPred p)
```

**equal** returns true if the given ranges are equal, that is, they have the same size and all elements at corresponding positions compare equal. Optionally, a binary predicate can be given to specify how two elements should compare equal.

### Lexicographical compare

```c++
template <parlay::Range R1, parlay::Range R2, class Compare>
bool lexicographical_compare(const R1& r1, const R2& r2, Compare less)
```

**lexicographical_compare** returns true if the first range compares lexicographically less than the second range. A range is considered lexicographically less than another if it is a prefix of the other or the first mismatched element compares less than the corresponding element in the other range.

### Unique

```c++
template<parlay::Range R>
auto unique(const R& r)
```

```c++
template <parlay::Range R, typename BinaryPredicate>
auto unique(const R& r, BinaryPredicate eq)
```

**unique** returns a sequence consisting of the elements of the given range that do not compare equal to the element preceding them. All elements in the output sequence maintain their original relative order. An optional binary predicate can be given to specify how two elements should compare equal.

### Min and max element

```c++
template <parlay::Range R>
auto min_element(R&& r)
```

```c++
template <parlay::Range R, typename Compare>
auto min_element(R&& r, Compare comp)
```

```c++
template <parlay::Range R>
auto max_element(R&& r)
```

```c++
template <parlay::Range R, typename Compare>
auto max_element(R&& r, Compare comp)
```

```c++
template <parlay::Range R>
auto minmax_element(R&& r)
```

```c++
template <parlay::Range R, typename Compare>
auto minmax_element(R&& r, Compare comp)
```

**min_element** and **max_element** return a pointer to the minimum or maximum element in the given range respectively. In the case of duplicates, the leftmost element is always selected. **minmax_element** returns a pair consisting of iterators to both the minimum and maximum element. An optional binary predicate can be supplied to dictate how two elements should compare.

### Reverse

```c++
template <parlay::Range R>
auto reverse(const R& r)
```

```c++
template <parlay::Range R>
auto reverse_inplace(R&& r)
```

**reverse** returns a new sequence consisting of the elements of the given range in reverse order. **reverse_inplace** reverses the elements of the given range.

### Rotate

```c++
template <parlay::Range R>
auto rotate(const R& r, size_t t)
```

**rotate** returns a new sequence consisting of the elements of the given range cyclically shifted left by `t` positions.

### Is sorted

```c++
template <parlay::Range R>
bool is_sorted(const R& r)
```

```c++
template <parlay::Range R, typename Compare>
bool is_sorted(const R& r, Compare comp)
```

```c++
template <parlay::Range R>
auto is_sorted_until(const R& r)
```

```c++
template <parlay::Range R, typename Compare>
auto is_sorted_until(const R& r, Compare comp)
```

**is_sorted** returns true if the given range is sorted. **is_sorted_until** returns an iterator to the first element of the range that is out of order, or the end iterator if the range is sorted. An optional binary predicate can be supplied to dictate how two elements should compare.

### Is partitioned

```c++
template <parlay::Range R, typename UnaryPred>
bool is_partitioned(const R& r, UnaryPred f)
```

**is_partitioned** returns true if the given range is partitioned with respect to the given unary predicate. A range is partitioned with respect to the given predicate if all elements for which the predicate returns true precede all of those for which it returns false.

### Remove

```c++
template<parlay::Range R, typename T>
auto remove(const R& r, const T& v)
```

```c++
template <parlay::Range R, typename UnaryPred>
auto remove_if(const R& r, UnaryPred pred)
```

**remove** returns a sequence consisting of the elements of the range `r` with any occurrences of `v` omitted. **remove_if** returns a sequence consisting of the elements of the range `r` with any elements such that the given predicate returns true omitted.

### Iota

```c++
template <typename Index>
auto iota(Index n)
```

**iota** returns a sequence of the given template type consisting of the integers from `0` to `n-1`.

### Flatten

```c++
template <parlay::Range R>
auto flatten(const R& r)
```

**flatten** takes a range of ranges and returns a single sequence consisting of the concatenation of each of the underlying ranges.

### Tokens

```c++
template <parlay::Range R, typename UnaryPred = decltype(parlay::is_whitespace)>
sequence<sequence<char>> tokens(const R& r, UnaryPred is_space = parlay::is_whitespace)
```

```c++
template <parlay::Range R, typename UnaryOp,
          typename UnaryPred = decltype(parlay::is_whitespace)>
auto map_tokens(const R& r, UnaryOp f, UnaryPred is_space = parlay::is_whitespace)
```

**tokens** splits the given character sequence into "words", which are the maximal contiguous subsequences that do not contain any whitespace characters. Optionally, a custom criteria for determining the delimiters (whitespace by default) can be given. For example, to split a sequence at occurrences of commas, one could provide a value of `[](char c) { return c == ','; }` for `is_space`.

**map_tokens** splits the given character sequence into words in the same manner, but instead of returning a sequence of all the words, it applies the given unary function `f` to every token. More specifically, `f` must be a function object that can take a `slice` of `value_type` equal to `char`. If `f` is a void function, i.e. returns nothing, then `map_tokens` returns nothing. Otherwise, if `f` returns values of type `T`, the result of `map_tokens` is a sequence of type `T` consisting of the results of evaluating `f` on each token. For example, to compute a sequence that contains the lengths of all of the words in a given input sequence, one could write

```c++
auto word_sizes = parlay::map_tokens(my_char_sequence, [](auto token) { return token.size(); });
```

In essence, `map_tokens` is just equivalent to `parlay::map(parlay::tokens(r), f)`, but is more efficient, because it avoids copying the tokens into new memory, and instead, applies the function `f` directly to the tokens in the original sequence.

### Split

```c++
template <parlay::Range R, parlay::Range BoolRange>
auto split_at(const R& r, const BoolRange& flags)
```

```c++
template <parlay::Range R, parlay::Range BoolRange, typename UnaryOp>
auto map_split_at(const R& r, const BoolRange& flags, UnaryOp f)
```

**split_at** splits the given sequence into contiguous subsequences. Specifically, the subsequences are the maximal contiguous subsequences between positions such that the corresponding element in `flags` is true. This means that the number of subsequences is always one greater than the number of true flags. Also note that if there are adjacent true flags, the result can contain empty subsequences. The elements at positions corresponding to true flags are not themselves included in any subsequence.

**map_split_at** similarly splits the given sequence into contiguous subsequences, but instead of returning these subsequences, it applies the given unary function `f` to each one. Specifically, `f` must be a function object that can take a slice of the input sequence. If `f` is a void function (i.e. returns nothing), then `map_split_at` returns nothing. Otherwise, `map_split_at` returns a sequence consisting of the results of applying `f` to each of the contiguous subsequences of `r`.

`map_split_at` is essentially equivalent to `parlay::map(parlay::split_at(r, flags), f)`, but is more efficient because the subsequences do not have to be copied into new memory, but are instead acted upon by `f` in place.
