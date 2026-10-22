#include <math/mat2.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

using namespace filament;

class MathReturnValues {
public:
    math::float2 getFloat2(int i) noexcept;
    math::float3 getFloat3(int i) noexcept;
    math::float4 getFloat4(int i) noexcept;

    math::double2 getDouble2(int i) noexcept;
    math::double3 getDouble3(int i) noexcept;
    math::double4 getDouble4(int i) noexcept;

    math::mat2f getMat2f(int i) noexcept;
    math::mat3f getMat3f(int i) noexcept;
    math::mat4f getMat4f(int i) noexcept;

    math::mat2 getMat2d(int i) noexcept;
    math::mat3 getMat3d(int i) noexcept;
    math::mat4 getMat4d(int i) noexcept;
};
