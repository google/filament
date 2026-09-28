#include <stdint.h>

class BitfieldSamplerTest {
public:
    enum class Filter : uint8_t {
        NEAREST = 0,
        LINEAR = 1
    };

    enum class Wrap : uint8_t {
        CLAMP = 0,
        REPEAT = 1
    };

    BitfieldSamplerTest() noexcept;
    BitfieldSamplerTest(Filter filter, Wrap wrap = Wrap::CLAMP) noexcept;

    void setFilter(Filter f) noexcept;
    Filter getFilter() const noexcept;

    void setWrap(Wrap w) noexcept;
    Wrap getWrap() const noexcept;

private:
    struct Params {
        Filter filter : 1;
        Wrap wrap : 1;
        uint32_t padding : 30;
    };
    Params mParams{};
};
