#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float3 ov [[color(0)]];
};

struct main0_in
{
    float3 iv [[user(locn0)]];
};

static inline __attribute__((always_inline))
void func0(thread float3& iv, thread float3& ov)
{
#line 104 "test.frag"
#line 106 "test.frag"
    if (iv.x < 0.0)
    {
#line 107 "test.frag"
        ov.x = 50.0;
    }
    else
    {
#line 109 "test.frag"
        ov.x = 60.0;
    }
#line 110 "test.frag"
}

static inline __attribute__((always_inline))
void func1(thread float3& iv, thread float3& ov)
{
#line 112 "test.frag"
#line 114 "test.frag"
    for (int i = 0; i < 4; i++)
    {
#line 116 "test.frag"
        func0(iv, ov);
#line 117 "test.frag"
        if (iv.y < 0.0)
        {
#line 118 "test.frag"
            ov.y = 70.0;
        }
        else
        {
#line 120 "test.frag"
            ov.y = 80.0;
        }
    }
#line 122 "test.frag"
}

static inline __attribute__((always_inline))
void func2(thread float3& iv, thread float3& ov)
{
#line 124 "test.frag"
#line 126 "test.frag"
    for (int i = 0; i < 4; i++)
    {
#line 128 "test.frag"
        func0(iv, ov);
#line 129 "test.frag"
        func1(iv, ov);
#line 130 "test.frag"
        if (iv.z < 0.0)
        {
#line 131 "test.frag"
            ov.z = 100.0;
        }
        else
        {
#line 133 "test.frag"
            ov.z = 120.0;
        }
    }
#line 135 "test.frag"
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
#line 137 "test.frag"
#line 139 "test.frag"
    func0(in.iv, out.ov);
#line 140 "test.frag"
    func1(in.iv, out.ov);
#line 141 "test.frag"
    func2(in.iv, out.ov);
#line 142 "test.frag"
    return out;
}

