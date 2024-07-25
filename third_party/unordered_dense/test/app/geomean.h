#pragma once

#include <cmath>
#include <iterator>
#include <utility>

template <typename It, typename Op>
[[nodiscard]] auto geomean(It begin, It end, Op op) -> double {
    double sum = 0.0;
    size_t count = 0;
    while (begin != end) {
        sum += std::log(op(*begin));
        ++begin;
        ++count;
    }

    sum /= static_cast<double>(count);
    return std::exp(sum);
}

template <typename Container, typename Op>
[[nodiscard]] auto geomean(Container&& c, Op op) -> double {
    return geomean(std::begin(c), std::end(c), std::move(op));
}
