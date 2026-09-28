#include <stdint.h>
#define UTILS_NOAPIGEN [[clang::annotate("apigen:skip")]]

namespace filament {

class Engine;

template<typename T>
struct UTILS_NOAPIGEN BuilderBase {};

class BuilderTest {
    struct BuilderDetails;
public:
    enum class Mode : uint8_t {
        DEFAULT = 0,
        FAST = 1
    };

    class Builder : public BuilderBase<BuilderDetails> {
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        Builder& intensity(float intensity) noexcept;
        Builder& mode(Mode mode) noexcept;
        Builder& parent(BuilderTest const* parent) noexcept;

        BuilderTest* build(Engine& engine);
    };

    float getIntensity() const noexcept;
    void setIntensity(float intensity) noexcept;
};

} // namespace filament
