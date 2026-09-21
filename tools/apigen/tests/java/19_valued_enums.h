#include <stdint.h>

class ValuedEnumsTest {
public:
    enum class FrameStatus : int32_t {
        SKIPPED_SPURIOUS = -2,
        SKIPPED_STALE = -1,
        ACCEPTED = 0
    };

    enum class BitFlags : uint32_t {
        NONE = 0,
        FLAG_A = 1,
        FLAG_B = 2,
        FLAG_C = 4
    };

    enum class AttachmentPoint : uint8_t {
        COLOR = 0,
        DEPTH = 1,
        STENCIL = 2,
        COLOR0 = 0,
        COLOR1 = 3
    };

    FrameStatus processFrame(FrameStatus status) noexcept;
    void setFlags(BitFlags flags) noexcept;
    BitFlags getFlags() noexcept;
    AttachmentPoint getAttachment() noexcept;
};
