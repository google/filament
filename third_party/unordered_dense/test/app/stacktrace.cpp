#if __GNUC__

#    include <fmt/format.h>

#    include <array>
#    include <csignal>
#    include <cstdio>
#    include <cstdlib>
#    include <execinfo.h>
#    include <unistd.h>

namespace {

void handle(int sig) {
    fmt::print(stderr, "Error: signal {}:\n", sig);
    auto ary = std::array<void*, 50>();

    // get void*'s for all entries on the stack
    auto size = backtrace(ary.data(), static_cast<int>(ary.size()));

    // print out all the frames to stderr
    fmt::print(stderr, "Error: signal {}. See stacktrace with\n", sig);
    fmt::print(stderr, "addr2line -Cafpie ./test/udm-test");
    for (size_t i = 0; i < static_cast<size_t>(size); ++i) {
        fmt::print(stderr, " {}", ary[i]);
    }
    exit(1); // NOLINT(concurrency-mt-unsafe)
}

class handler {
public:
    handler() {
        (void)signal(SIGTERM, handle);
    }
};

auto const global_h = handler();

} // namespace

#endif