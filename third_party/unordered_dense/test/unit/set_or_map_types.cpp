#include <ankerl/unordered_dense.h>

template <typename T>
using detect_has_mapped_type = typename T::mapped_type;

using map1_t = ankerl::unordered_dense::map<int, double>;
static_assert(std::is_same_v<double, map1_t::mapped_type>);
static_assert(ankerl::unordered_dense::detail::is_detected_v<detect_has_mapped_type, map1_t>);

using map2_t = ankerl::unordered_dense::segmented_map<int, double>;
static_assert(std::is_same_v<double, map2_t::mapped_type>);
static_assert(ankerl::unordered_dense::detail::is_detected_v<detect_has_mapped_type, map2_t>);

using set1_t = ankerl::unordered_dense::set<int>;
static_assert(!ankerl::unordered_dense::detail::is_detected_v<detect_has_mapped_type, set1_t>);

using set2_t = ankerl::unordered_dense::segmented_set<int>;
static_assert(!ankerl::unordered_dense::detail::is_detected_v<detect_has_mapped_type, set2_t>);
