#include <math/mat4.h>

#include <stdint.h>

class EnumsAndStaticMethodsTest {
public:
    enum class Mode : int {
        FIRST = 0,
        SECOND = 1,
        THIRD = 2
    };

    enum class Status : uint32_t {
        IDLE,
        BUSY,
        DONE
    };

    // 1. Instance method taking and returning enum
    Mode setAndGetMode(Mode m) noexcept;

    // 2. Instance throwing method taking enum
    void setModeThrowing(Mode m);

    // 3. Static method with primitive args and return
    static int32_t addNumbers(int32_t a, int32_t b) noexcept;

    // 4. Static method throwing
    static void executeStaticThrowing(Mode m);

    // 5. Static method returning math type
    static filament::math::mat4 computeMatrix(Mode m, double scale) noexcept;

    // 6. Static method returning enum
    static Status getGlobalStatus() noexcept;
};
