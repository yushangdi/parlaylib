
#ifndef PARLAY_INTERNAL_DELAYED_SCAN_H
#define PARLAY_INTERNAL_DELAYED_SCAN_H

#include <functional>
#include <type_traits>
#include <utility>

#include "common.h"

#include "../sequence_ops.h"

#include "../../monoid.h"
#include "../../range.h"
#include "../../slice.h"

namespace parlay {
namespace internal {
namespace delayed {

template<typename UnderlyingView, bool Inclusive, typename BinaryOperator, typename T>
struct block_delayed_scan_t :
    public block_iterable_view_base<UnderlyingView, block_delayed_scan_t<UnderlyingView, Inclusive, BinaryOperator, T>> {

 private:
  using base = block_iterable_view_base<UnderlyingView, block_delayed_scan_t<UnderlyingView, Inclusive, BinaryOperator, T>>;
  using base::base_view;

  using base_ref_type = range_reference_type_t<UnderlyingView>;

 public:
  using reference = T;
  using value_type = T;

  static_assert(std::is_move_constructible_v<T>);

  static_assert(std::is_invocable_v<BinaryOperator, T&&, base_ref_type>);
  static_assert(std::is_invocable_v<BinaryOperator, T&&, T&&>);
  static_assert(std::is_invocable_v<BinaryOperator, base_ref_type, base_ref_type>);

  static_assert(std::is_convertible_v<std::invoke_result_t<BinaryOperator, T&&, base_ref_type>, T>);
  static_assert(std::is_convertible_v<std::invoke_result_t<BinaryOperator, T&&, T&&>, T>);
  static_assert(std::is_convertible_v<std::invoke_result_t<BinaryOperator, base_ref_type, base_ref_type>, T>);

  template<typename UV>
  block_delayed_scan_t(UV&& v, BinaryOperator f, T identity)
      : base(std::forward<UV>(v)), op(std::move(f)), total(identity) {

    size_t n_blocks = num_blocks(base_view());

    if (!(n_blocks == 1 && Inclusive)) {
      block_sums = parlay::internal::tabulate(n_blocks+1, [&](size_t i) {
        T result = identity;
        if (i == n_blocks) return result;
        for (auto it = begin_block(base_view(), i); it != end_block(base_view(), i); ++it) {
          result = op(std::move(result), *it);
        }
        return result;
      });
      total = parlay::internal::scan_inplace(make_slice(block_sums), parlay::make_monoid(*(op.get()), identity));
    }
    else {
      block_sums = sequence<T>(1, identity);
    }

    // If inclusive, each block needs to initially include its first value
    if constexpr (Inclusive) {
      parallel_for(0, n_blocks, [&](size_t i) {
        block_sums[i] = op(std::move(block_sums[i]), *begin_block(base_view(), i));
      });
    }
  }

  template<bool Const>
  struct iterator_t {
   private:
    using parent_type = std::conditional_t<Const,
        typename std::add_const_t<block_delayed_scan_t<UnderlyingView, Inclusive, BinaryOperator, T>>,
                                  block_delayed_scan_t<UnderlyingView, Inclusive, BinaryOperator, T>>;
    using parent_ptr = std::add_pointer_t<parent_type>;
    using base_view_type = std::conditional_t<Const,
        typename std::add_const_t<std::remove_reference_t<UnderlyingView>>,
        typename                  std::remove_reference_t<UnderlyingView>>;
    using base_iterator_type = range_iterator_type_t<base_view_type>;

   public:
    using iterator_category = std::forward_iterator_tag;
    using reference = T;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = void;

    T operator*() const { return value; }

    iterator_t& operator++() {
      if constexpr (Inclusive) {
        if (++it != std::end(parent->base_view())) {
          value = (parent->op)(std::move(value), *it);
        }
      }
      else {
        value = (parent->op)(std::move(value), *it);
        ++it;
      }
      return *this;
    }
    iterator_t operator++(int) { auto tmp = *this; ++(*this); return tmp; }

    friend bool operator==(const iterator_t& x, const iterator_t& y) { return x.it == y.it; }
    friend bool operator!=(const iterator_t& x, const iterator_t& y) { return x.it != y.it; }

   private:
    friend parent_type;

    iterator_t() : value{}, it{}, parent(nullptr) {}
    iterator_t(T value_, base_iterator_type it_, parent_ptr parent_)
        : value(std::move(value_)), it(it_), parent(parent_) {}

    T value;
    base_iterator_type it;
    parent_ptr parent;
  };

  using iterator = iterator_t<false>;
  using const_iterator = iterator_t<true>;

  // Returns the total sum of the range
  template<typename U = T, typename = std::enable_if_t<!Inclusive, U>>
  auto get_total() const { return total; }

  // Returns the number of blocks
  auto get_num_blocks() const { return num_blocks(base_view()); }

  // Return an iterator pointing to the beginning of block i
  auto get_begin_block(size_t i) { return iterator(block_sums[i], begin_block(base_view(), i), this); }
  auto get_begin_block(size_t i) const { return const_iterator(block_sums[i], begin_block(base_view(), i), this); }

  [[nodiscard]] size_t size() const { return base_view().size(); }

 private:
  copyable_function_wrapper<BinaryOperator> op;
  T total;
  sequence<T> block_sums;
};

template<typename T, typename UnderlyingView, typename BinaryOperator>
auto scan(UnderlyingView&& v, BinaryOperator f, T initial) {
  auto s = block_delayed_scan_t<UnderlyingView, false, BinaryOperator, T>
      (std::forward<UnderlyingView>(v), std::move(f), std::move(initial));
  return std::make_pair(std::move(s), s.get_total());
}

template<typename UnderlyingView, typename BinaryOperator>
auto scan(UnderlyingView&& v, BinaryOperator f) {
  return scan(std::forward<UnderlyingView>(v), std::move(f), range_value_type_t<UnderlyingView>{});
}

template<typename UnderlyingView>
auto scan(UnderlyingView&& v) {
  return scan(std::forward<UnderlyingView>(v), std::plus<>{}, range_value_type_t<UnderlyingView>{});
}

template<typename T, typename UnderlyingView, typename BinaryOperator>
auto scan_inclusive(UnderlyingView&& v, BinaryOperator f, T identity) {
  return block_delayed_scan_t<UnderlyingView, true, BinaryOperator, T>
      (std::forward<UnderlyingView>(v), std::move(f), std::move(identity));
}

template<typename UnderlyingView, typename BinaryOperator>
auto scan_inclusive(UnderlyingView&& v, BinaryOperator f) {
  return scan_inclusive(std::forward<UnderlyingView>(v), std::move(f), range_value_type_t<UnderlyingView>{});
}

template<typename UnderlyingView>
auto scan_inclusive(UnderlyingView&& v) {
  return scan_inclusive(std::forward<UnderlyingView>(v), std::plus<>{}, range_value_type_t<UnderlyingView>{});
}

}  // namespace delayed
}  // namespace internal
}  // namespace parlay

#endif  // PARLAY_INTERNAL_DELAYED_SCAN_H
