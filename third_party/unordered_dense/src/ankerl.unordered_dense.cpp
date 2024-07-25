module;

// see https://github.com/fmtlib/fmt/blob/master/src/fmt.cc

// Put all implementation-provided headers into the global module fragment
// to prevent attachment to this module.

#include <array>            // for array
#include <cstdint>          // for uint64_t, uint32_t, uint8_t, UINT64_C
#include <cstring>          // for size_t, memcpy, memset
#include <functional>       // for equal_to, hash
#include <initializer_list> // for initializer_list
#include <iterator>         // for pair, distance
#include <limits>           // for numeric_limits
#include <memory>           // for allocator, allocator_traits, shared_ptr
#include <stdexcept>        // for out_of_range
#include <string>           // for basic_string
#include <string_view>      // for basic_string_view, hash
#include <tuple>            // for forward_as_tuple
#include <type_traits>      // for enable_if_t, declval, conditional_t, ena...
#include <utility>          // for forward, exchange, pair, as_const, piece...
#include <vector>           // for vector
#if defined(__has_include)
#    if __has_include(<memory_resource>)
#        include <memory_resource> // for polymorphic_allocator
#    elif __has_include(<experimental/memory_resource>)
#        include <experimental/memory_resource> // for polymorphic_allocator
#    endif
#endif
#if defined(_MSC_VER) && defined(_M_X64)
#    include <intrin.h>
#    pragma intrinsic(_umul128)
#endif

export module ankerl.unordered_dense;

#define ANKERL_UNORDERED_DENSE_EXPORT export

#include "ankerl/unordered_dense.h"
