#include <stdint.h>
#define UTILS_NOAPIGEN [[clang::annotate("apigen:skip")]]

namespace filament {

template<typename T>
struct UTILS_NOAPIGEN BuilderBase {};

class NestedAggregatesTest {
    struct BuilderDetails;
public:
    struct ShadowOptions {
        struct Vsm {
            bool elvsm = false;
            float blurWidth = 0.0f;
        };

        uint32_t mapSize = 1024;
        float cascadeSplitPositions[3] = { 0.125f, 0.25f, 0.50f };
        Vsm vsm;
        float constantBias = 0.001f;
    };

    class Builder : public BuilderBase<BuilderDetails> {
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        Builder& shadowOptions(const ShadowOptions& options) noexcept;
    };

    void setShadowOptions(int i, const ShadowOptions& options) noexcept;
};

} // namespace filament
