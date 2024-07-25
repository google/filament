#pragma once

#include <chrono>

namespace ui {

class periodic {
    std::chrono::steady_clock::time_point m_next{};
    std::chrono::steady_clock::duration m_interval{};

public:
    explicit periodic(std::chrono::steady_clock::duration interval);
    explicit operator bool();
};

} // namespace ui
