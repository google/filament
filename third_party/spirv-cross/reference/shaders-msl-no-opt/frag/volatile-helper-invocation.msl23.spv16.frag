#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
void func(thread float& FragColor, thread bool& gl_HelperInvocation)
{
    bool _14 = gl_HelperInvocation;
    float _17 = float(_14);
    FragColor = _17;
    gl_HelperInvocation = true, discard_fragment();
    bool _18 = gl_HelperInvocation;
    float _19 = float(_18);
    FragColor = _19;
}

fragment main0_out main0()
{
    main0_out out = {};
    bool gl_HelperInvocation = {};
    gl_HelperInvocation = simd_is_helper_thread();
    func(out.FragColor, gl_HelperInvocation);
    return out;
}

