#include <fmt/format.h>

#include <cstdio>

namespace test {

template <typename... Args>
constexpr void print(fmt::format_string<Args...> f, Args&&... args) {
    fmt::print(f, std::forward<Args>(args)...);
    (void)std::fflush(stdout);
}

#ifndef ENABLE_LOG_LINE
#    define LOG_LINE(what)
#else
#    define LOG_LINE(what) ::test::print("{}({:3}) {}\n", __FILE__, __LINE__, what)
#endif

} // namespace test
