#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <type_traits>

namespace fuzz {

// Helper to provide a little bit more convenient interface than FuzzedDataProvider itself
class provider {
    uint8_t const* m_data;
    size_t m_remaining_bytes;

    // Reads one byte and returns a bool, or false when no data remains.
    [[nodiscard]] inline auto consume_bool() -> bool {
        return (1U & consume_integral<uint8_t>()) != 0U;
    }

    // Returns a number in the range [Type's min, Type's max]. The value might
    // not be uniformly distributed in the given range. If there's no input data
    // left, always returns |min|.
    template <typename T>
    [[nodiscard]] auto consume_integral() -> T {
        return consume_integral_in_range(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }

    // Returns a number in the range [min, max] by consuming bytes from the
    // input data. The value might not be uniformly distributed in the given
    // range. If there's no input data left, always returns |min|. |min| must
    // be less than or equal to |max|.
    template <typename T>
    [[nodiscard]] auto consume_integral_in_range(T min, T max) -> T {
        static_assert(std::is_integral<T>::value, "An integral type is required.");
        static_assert(sizeof(T) <= sizeof(uint64_t), "Unsupported integral type.");

        if (min > max) {
            std::abort();
        }

        // Use the biggest type possible to hold the range and the result.
        uint64_t range = static_cast<uint64_t>(max) - static_cast<uint64_t>(min);
        uint64_t result = 0;
        size_t offset = 0;

        while (offset < sizeof(T) * CHAR_BIT && (range >> offset) > 0 && m_remaining_bytes != 0) {
            // Pull bytes off the end of the seed data. Experimentally, this seems to
            // allow the fuzzer to more easily explore the input space. This makes
            // sense, since it works by modifying inputs that caused new code to run,
            // and this data is often used to encode length of data read by
            // |ConsumeBytes|. Separating out read lengths makes it easier modify the
            // contents of the data that is actually read.
            --m_remaining_bytes;
            result = (result << CHAR_BIT) | m_data[m_remaining_bytes];
            offset += CHAR_BIT;
        }

        // Avoid division by 0, in case |range + 1| results in overflow.
        if (range != std::numeric_limits<decltype(range)>::max()) {
            result = result % (range + 1);
        }

        return static_cast<T>(static_cast<uint64_t>(min) + result);
    }

    inline void advance_unchecked(size_t num_bytes) {
        m_data += num_bytes;
        m_remaining_bytes -= num_bytes;
    }

    // Returns a std::string of length from 0 to |max_length|. When it runs out of
    // input data, returns what remains of the input. Designed to be more stable
    // with respect to a fuzzer inserting characters than just picking a random
    // length and then consuming that many bytes with |ConsumeBytes|.
    [[nodiscard]] inline auto consume_random_length_string(size_t max_length) -> std::string {
        // Reads bytes from the start of |data_ptr_|. Maps "\\" to "\", and maps "\"
        // followed by anything else to the end of the string. As a result of this
        // logic, a fuzzer can insert characters into the string, and the string
        // will be lengthened to include those new characters, resulting in a more
        // stable fuzzer than picking the length of a string independently from
        // picking its contents.
        std::string result;

        // Reserve the anticipated capacity to prevent several reallocations.
        result.reserve(std::min(max_length, m_remaining_bytes));
        for (size_t i = 0; i < max_length && m_remaining_bytes != 0; ++i) {
            auto next = m_data[0];
            advance_unchecked(1);
            if (next == '\\' && m_remaining_bytes != 0) {
                next = m_data[0];
                advance_unchecked(1);
                if (next != '\\') {
                    break;
                }
            }
            result += static_cast<char>(next);
        }

        result.shrink_to_fit();
        return result;
    }

    provider(provider const&) = default;
    auto operator=(provider const&) -> provider& = default;

public:
    provider(provider&&) = default;
    auto operator=(provider&&) -> provider& = default;
    ~provider() = default;

    [[nodiscard]] auto copy() const -> provider {
        return *this;
    }

    inline explicit provider(void const* data, size_t size)
        : m_data(reinterpret_cast<uint8_t const*>(data)) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */
        , m_remaining_bytes(size) /* NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) */ {}

    // random number in inclusive range [min, max]
    template <typename T>
    auto range(T min, T max) -> T {
        return consume_integral_in_range<T>(min, max);
    }

    template <typename T>
    auto bounded(T max_exclusive) -> T {
        if (0 == max_exclusive) {
            return {};
        }
        return consume_integral_in_range<T>(0, max_exclusive - 1);
    }

    template <typename T>
    auto integral() -> T {
        if constexpr (std::is_same_v<bool, T>) {
            return consume_bool();
        } else {
            return consume_integral<T>();
        }
    }

    inline auto string(size_t max_length) -> std::string {
        return consume_random_length_string(max_length);
    }

    template <typename... Args>
    auto pick(Args&&... args) -> std::common_type_t<decltype(args)...>& {
        static constexpr auto num_ops = sizeof...(args);

        auto idx = size_t{};
        auto const chosen_idx = consume_integral_in_range<size_t>(0, num_ops - 1);
        std::common_type_t<decltype(args)...>* result = nullptr;
        ((idx++ == chosen_idx ? (result = &args, true) : false) || ...);
        return *result;
    }

    template <typename... Ops>
    void repeat_oneof(Ops&&... op) {
        static constexpr auto num_ops = sizeof...(op);

        do {
            if constexpr (num_ops == 1) {
                (op(), ...);
            } else {
                auto chosen_op_idx = range<size_t>(0, num_ops - 1);
                auto op_idx = size_t{};
                ((op_idx++ == chosen_op_idx ? op() : void()), ...);
            }
        } while (0 != m_remaining_bytes);
    }

    template <typename... Ops>
    void limited_repeat_oneof(size_t min, size_t max, Ops&&... op) {
        static constexpr auto num_ops = sizeof...(op);

        size_t const num_evaluations = consume_integral_in_range(min, max);
        for (size_t i = 0; i < num_evaluations; ++i) {
            if constexpr (num_ops == 1) {
                (op(), ...);
            } else {
                auto chosen_op_idx = range<size_t>(0, num_ops - 1);
                auto op_idx = size_t{};
                ((op_idx++ == chosen_op_idx ? op() : void()), ...);
            }
            if (m_remaining_bytes == 0) {
                return;
            }
        }
    }

    [[nodiscard]] auto has_remaining_bytes() const -> bool {
        return 0U != m_remaining_bytes;
    }

    static inline void require(bool b) {
        if (!b) {
            std::abort();
        }
    }
};

} // namespace fuzz
