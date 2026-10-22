/**
 * MIT License
 *
 * Copyright (c) 2017 Thibaut Goetghebuer-Planchon <tessil@gmx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <tsl/robin_map.h>

#include <boost/test/unit_test.hpp>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "utils.h"

static std::size_t nb_custom_allocs = 0;

template <typename T>
class custom_allocator {
 public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using propagate_on_container_move_assignment = std::true_type;

  template <typename U>
  struct rebind {
    using other = custom_allocator<U>;
  };

  custom_allocator() = default;
  custom_allocator(const custom_allocator&) = default;

  template <typename U>
  custom_allocator(const custom_allocator<U>&) {}

  pointer address(reference x) const noexcept { return &x; }

  const_pointer address(const_reference x) const noexcept { return &x; }

  pointer allocate(size_type n, const void* /*hint*/ = 0) {
    nb_custom_allocs++;

    pointer ptr = static_cast<pointer>(std::malloc(n * sizeof(T)));
    if (ptr == nullptr) {
#ifdef TSL_RH_NO_EXCEPTIONS
      std::abort();
#else
      throw std::bad_alloc();
#endif
    }

    return ptr;
  }

  void deallocate(T* p, size_type /*n*/) { std::free(p); }

  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(value_type);
  }

  template <typename U, typename... Args>
  void construct(U* p, Args&&... args) {
    ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
  }

  template <typename U>
  void destroy(U* p) {
    p->~U();
  }
};

template <class T, class U>
bool operator==(const custom_allocator<T>&, const custom_allocator<U>&) {
  return true;
}

template <class T, class U>
bool operator!=(const custom_allocator<T>&, const custom_allocator<U>&) {
  return false;
}

// TODO Avoid overloading new to check number of global new
// static std::size_t nb_global_new = 0;
// void* operator new(std::size_t sz) {
//     nb_global_new++;
//     return std::malloc(sz);
// }
//
// void operator delete(void* ptr) noexcept {
//     std::free(ptr);
// }

BOOST_AUTO_TEST_SUITE(test_custom_allocator)

BOOST_AUTO_TEST_CASE(test_custom_allocator_1) {
  //    nb_global_new = 0;
  nb_custom_allocs = 0;

  tsl::robin_map<int, int, std::hash<int>, std::equal_to<int>,
                 custom_allocator<std::pair<int, int>>>
      map;

  const int nb_elements = 1000;
  for (int i = 0; i < nb_elements; i++) {
    map.insert({i, i * 2});
  }

  BOOST_CHECK_NE(nb_custom_allocs, 0);
  //    BOOST_CHECK_EQUAL(nb_global_new, 0);
}

BOOST_AUTO_TEST_SUITE_END()
