#pragma once

#include <fmt/ostream.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>

// Source: https://github.com/bitcoin/bitcoin/blob/master/src/memusage.h#L41-L61
static inline auto malloc_usage(size_t alloc) -> size_t {
    static_assert(sizeof(void*) == 8 || sizeof(void*) == 4);

    // Measured on libc6 2.19 on Linux.
    if constexpr (sizeof(void*) == 8U) {
        return ((alloc + 31U) >> 4U) << 4U;
    } else {
        return ((alloc + 15U) >> 3U) << 3U;
    }
}

class counts_for_allocator {
    struct measurement_internal {
        std::chrono::steady_clock::time_point m_tp{};
        size_t m_diff{};
    };

    struct measurement {
        std::chrono::steady_clock::duration m_duration{};
        size_t m_num_bytes_allocated{};
    };

    std::vector<measurement_internal> m_measurements{};
    std::chrono::steady_clock::time_point m_start = std::chrono::steady_clock::now();

    template <typename Op>
    void each_measurement(Op op) const {
        auto total_bytes = size_t();
        auto const start_time = m_start;
        for (auto const& m : m_measurements) {
            bool is_add = true;
            size_t bytes = m.m_diff;
            if (bytes > (0U - bytes)) {
                // negative number
                is_add = false;
                bytes = 0U - bytes;
            }

            if (is_add) {
                total_bytes += malloc_usage(bytes);
            } else {
                total_bytes -= malloc_usage(bytes);
            }
            op(measurement{m.m_tp - start_time, total_bytes});
        }
    }

public:
    void add(size_t count) {
        m_measurements.emplace_back(measurement_internal{std::chrono::steady_clock::now(), count});
    }

    void sub(size_t count) {
        // overflow, but it's ok
        m_measurements.emplace_back(measurement_internal{std::chrono::steady_clock::now(), 0U - count});
    }

    void save(std::filesystem::path const& filename) const {
        auto fout = std::ofstream(filename);
        each_measurement([&](measurement m) {
            fmt::print(fout, "{}; {}\n", std::chrono::duration<double>(m.m_duration).count(), m.m_num_bytes_allocated);
        });
    }

    [[nodiscard]] auto size() const -> size_t {
        return m_measurements.size();
    }

    void reset() {
        m_measurements.clear();
        m_start = std::chrono::steady_clock::now();
    }
};

/**
 * Forwards all allocations/deallocations to the counts
 */
template <class T>
class counting_allocator {
    counts_for_allocator* m_counts;

    template <typename U>
    friend class counting_allocator;

public:
    using value_type = T;

    /**
     * Not explicit so we can easily construct it with the correct resource
     */
    counting_allocator(counts_for_allocator* counts) noexcept // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        : m_counts(counts) {}

    /**
     * Not explicit so we can easily construct it with the correct resource
     */
    template <class U>
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    counting_allocator(counting_allocator<U> const& other) noexcept
        : m_counts(other.m_counts) {}

    counting_allocator(counting_allocator const& other) noexcept = default;
    counting_allocator(counting_allocator&& other) noexcept = default;
    auto operator=(counting_allocator const& other) noexcept -> counting_allocator& = default;
    auto operator=(counting_allocator&& other) noexcept -> counting_allocator& = default;
    ~counting_allocator() = default;

    auto allocate(size_t n) -> T* {
        m_counts->add(sizeof(T) * n);
        return std::allocator<T>{}.allocate(n);
    }

    void deallocate(T* p, size_t n) noexcept {
        m_counts->sub(sizeof(T) * n);
        std::allocator<T>{}.deallocate(p, n);
    }

    template <class U>
    friend auto operator==(counting_allocator const& a, counting_allocator<U> const& b) noexcept -> bool {
        return a.m_counts == b.m_counts;
    }

    template <class U>
    friend auto operator!=(counting_allocator const& a, counting_allocator<U> const& b) noexcept -> bool {
        return a.m_counts != b.m_counts;
    }
};
