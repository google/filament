#pragma once

#include <cstddef>     // for size_t
#include <string>      // for string, basic_string
#include <string_view> // for string_view
#include <vector>      // for vector

namespace ui {

class progress_bar {
    size_t m_width;
    size_t m_total;
    std::vector<std::string> m_symbols;

public:
    progress_bar(size_t width, size_t total, std::string_view symbols = "⡀ ⡄ ⡆ ⡇ ⡏ ⡟ ⡿ ⣿");

    auto operator()(size_t current) const -> std::string;
};

} // namespace ui
