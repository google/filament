#include <utils/bitset.h>
#include <utils/CString.h>
#include <utils/ImmutableCString.h>
#include <utils/StaticString.h>
#include <utils/tribool.h>

#include <chrono>
#include <optional>
#include <string_view>

class StringsAndPrimitivesTest {
public:
    // 1. Strings
    bool hasParameter(std::string_view name) const noexcept;
    void setName(const utils::CString& name) noexcept;
    void setStaticName(utils::StaticString name) noexcept;
    utils::ImmutableCString getName() const noexcept;
    const char* getTag() const noexcept;

    // 2. Chrono
    void setLatency(std::chrono::nanoseconds latency) noexcept;
    std::chrono::steady_clock::time_point getPresentationTime() const noexcept;

    // 3. Tribool and Optional
    void setTriMode(utils::tribool mode) noexcept;
    utils::tribool getTriMode() const noexcept;
    void setOptionalFlag(std::optional<bool> flag) noexcept;
    std::optional<bool> getOptionalFlag() const noexcept;

    // 4. Bitset
    void setFlags(utils::bitset<uint32_t> flags) noexcept;
    utils::bitset<uint32_t> getFlags() const noexcept;
};
