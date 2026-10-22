#include <math/vec3.h>

class ExceptionsTest {
public:
    void doNoexcept(int a) noexcept;
    int getNoexcept(int a) const noexcept;
    void doThrow(int a);
    int getThrow(int a) const;
    filament::math::float3 getVectorNoexcept(int a) const noexcept;
    filament::math::float3 getVectorThrow(int a) const;
};
