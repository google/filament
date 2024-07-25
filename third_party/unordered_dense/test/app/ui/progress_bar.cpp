#include "progress_bar.h"

#include <algorithm> // for min

namespace {

auto split(std::string_view symbols, char sep) -> std::vector<std::string> {
    auto s = std::vector<std::string>();
    while (true) {
        auto idx = symbols.find(sep);
        if (idx == std::string_view::npos) {
            break;
        }
        s.emplace_back(symbols.substr(0, idx));
        symbols.remove_prefix(idx + 1);
    }
    s.emplace_back(symbols);
    return s;
}

} // namespace

namespace ui {

progress_bar::progress_bar(size_t width, size_t total, std::string_view symbols)
    : m_width(width)
    , m_total(total)
    , m_symbols(split(symbols, ' ')) {}

auto progress_bar::operator()(size_t current) const -> std::string {
    auto const total_states = m_width * m_symbols.size() + 1;
    auto const current_state = total_states * current / m_total;
    std::string str;
    auto num_full = std::min(m_width, current_state / m_symbols.size());
    for (size_t i = 0; i < num_full; ++i) {
        str += m_symbols.back();
    }

    if (num_full < m_width) {
        auto remaining = current_state - num_full * m_symbols.size();
        if (0U != remaining) {
            str += m_symbols[remaining - 1];
        }

        auto num_fillers = m_width - num_full - (0U == remaining ? 0 : 1);
        for (size_t i = 0; i < num_fillers; ++i) {
            str.push_back(' ');
        }
    }
    return str;
}

} // namespace ui
