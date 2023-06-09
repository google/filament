#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
float _mat3(thread const float& a)
{
    return a + 1.0;
}

static inline __attribute__((always_inline))
float _RESERVED_IDENTIFIER_FIXUP_gl_Foo(thread const int& a)
{
    return float(a) + 1.0;
}

fragment main0_out main0()
{
    main0_out out = {};
    float param = 2.0;
    int param_1 = 4;
    out.FragColor = _mat3(param) + _RESERVED_IDENTIFIER_FIXUP_gl_Foo(param_1);
    return out;
}

