#include <utils/compiler.h>
#include <utils/Slice.h>

#include <math/mat2.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

using namespace filament;

class MathParams {
public:
    /**
     * @param i index
     * @param v value
     */
    void setFloat2(int i, math::float2 v) noexcept;
    void setFloat3(int i, math::float3 v) noexcept;
    void setFloat4(int i, math::float4 v) noexcept;

    void setDouble2(int i, math::double2 v) noexcept;
    void setDouble3(int i, math::double3 v) noexcept;
    void setDouble4(int i, math::double4 v) noexcept;

    void setMat2f(int i, math::mat2f v) noexcept;
    void setMat3f(int i, math::mat3f v) noexcept;
    void setMat4f(int i, math::mat4f v) noexcept;

    void setMat2d(int i, math::mat2 v) noexcept;
    void setMat3d(int i, math::mat3 v) noexcept;
    void setMat4d(int i, math::mat4 v) noexcept;

    void setFloat4ptr_nullable(math::float4 const* UTILS_NULLABLE p) noexcept;
    void setFloat4ptr_nonnull(math::float4 const* UTILS_NONNULL p) noexcept;
    void setFloat4ptr(math::float4 const* p) noexcept;

    void setMat4Array(utils::Slice<const math::mat4> p) noexcept;
    void setMat3fArray(utils::Slice<const math::mat3f> p) noexcept;
    void cvtVecsArray(int i,
        utils::Slice<const math::float2> s, 
        utils::Slice<const math::float3> d) noexcept;
};
