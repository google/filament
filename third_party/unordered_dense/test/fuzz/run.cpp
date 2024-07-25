#include <fuzz/run.h>

#include <app/doctest.h>
#include <fmt/format.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace fuzz::detail {

namespace {

[[nodiscard]] constexpr auto is_alpha(char c) -> bool {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

[[nodiscard]] constexpr auto is_digit(char c) -> bool {
    return c >= '0' && c <= '9';
}

[[nodiscard]] constexpr auto is_alnum(char c) -> bool {
    return is_alpha(c) || is_digit(c);
}

[[nodiscard]] constexpr auto contains(std::string_view haystack, char needle) -> bool {
    return std::string_view::npos != haystack.find_first_of(needle);
}

[[nodiscard]] constexpr auto is_valid_filename(std::string_view name) -> bool {
    using namespace std::literals;
    for (auto c : name) {
        if (!is_alnum(c) && !contains("_-+", c)) {
            return false;
        }
    }
    return true;
}

auto env(char const* varname) -> std::optional<std::string> {
#ifdef _MSC_VER
    char* pValue = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&pValue, &len, varname);
    if (err || nullptr == pValue) {
        return {};
    }
    auto str = std::string(pValue);
    free(pValue);
    return str;
#else
    char const* val = std::getenv(varname); // NOLINT(concurrency-mt-unsafe,clang-analyzer-cplusplus.StringChecker)
    if (nullptr == val) {
        return {};
    }
    return val;
#endif
}

[[nodiscard]] auto read_file(std::filesystem::path const& p) -> std::optional<std::string> {
    auto f = std::ifstream(p);
    if (!f) {
        return {};
    }
    auto content = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (f.bad()) {
        return {};
    }
    return content;
}

[[nodiscard]] auto find_fuzz_corpus_base_dir() -> std::optional<std::filesystem::path> {
    auto corpus_base_dir = env("FUZZ_CORPUS_BASE_DIR");
    if (corpus_base_dir) {
        return corpus_base_dir.value();
    }

    auto p = std::filesystem::current_path();
    while (true) {
        auto const filename = p / ".fuzz-corpus-base-dir";
        // INFO(fmt::format("trying '{}'", filename.string()));
        if (std::filesystem::exists(filename)) {
            if (auto file_content = read_file(p / ".fuzz-corpus-base-dir"); file_content) {
                auto f = std::filesystem::path(file_content.value()).make_preferred();
                // INFO(fmt::format("got it! p='{}, f='{}', p/f='{}'\n", p.string(), f.string(), (p / f).string()));
                return p / f;
            }
            // could not read file
            throw std::runtime_error(fmt::format("could not read '{}'", filename.string()));
        }
        if (p == p.root_path()) {
            return {};
        }
        p = p.parent_path();
    }
}

} // namespace

void evaluate_corpus(std::function<void(provider)> const& op) {
    if (!is_valid_filename(doctest::current_test_name())) {
        throw std::runtime_error("test case name needs to be a valid filename. only [a-zA-Z0-9_-+] are allowed");
    }

    // 2 ways

    auto corpus_base_dir = find_fuzz_corpus_base_dir();
    if (!corpus_base_dir) {
        throw std::runtime_error("could not find corpus base dir :-(");
    }

    auto path = std::filesystem::path(corpus_base_dir.value()) / doctest::current_test_name();
    INFO("path=\"" << path.string() << "\"");
    auto num_files = size_t();
    for (auto const& dir_entry : std::filesystem::directory_iterator(path)) {
        ++num_files;
        auto const& test_file = dir_entry.path();
        CAPTURE(test_file);

        auto f = std::ifstream(test_file);
        auto content = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        op(provider(content.data(), content.size()));
    }
    REQUIRE(num_files > 1);
}

} // namespace fuzz::detail
