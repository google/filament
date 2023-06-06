#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Vert
{
    float a;
    float b;
};

struct Foo
{
    float c;
    float d;
};

struct main0_out
{
    float m_3_a [[user(locn0)]];
    float m_3_b [[user(locn1)]];
    float foo_c [[user(locn2)]];
    float foo_d [[user(locn3)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    Vert _3 = Vert{ 0.0, 0.0 };
    Foo foo = Foo{ 0.0, 0.0 };
    out.gl_Position = float4(0.0);
    out.m_3_a = _3.a;
    out.m_3_b = _3.b;
    out.foo_c = foo.c;
    out.foo_d = foo.d;
    return out;
}

