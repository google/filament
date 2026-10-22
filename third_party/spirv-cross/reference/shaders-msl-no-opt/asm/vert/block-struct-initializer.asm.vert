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
    float m_4_a [[user(locn0)]];
    float m_4_b [[user(locn1)]];
    float foo_c [[user(locn2)]];
    float foo_d [[user(locn3)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    Vert _4 = Vert{ 0.0, 0.0 };
    Foo foo = Foo{ 0.0, 0.0 };
    out.gl_Position = float4(0.0);
    out.m_4_a = _4.a;
    out.m_4_b = _4.b;
    out.foo_c = foo.c;
    out.foo_d = foo.d;
    return out;
}

