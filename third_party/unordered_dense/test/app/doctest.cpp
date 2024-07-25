#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

namespace doctest {

[[nodiscard]] auto current_test_name() -> char const* {
    return doctest::detail::g_cs->currentTest->m_name;
}

} // namespace doctest
