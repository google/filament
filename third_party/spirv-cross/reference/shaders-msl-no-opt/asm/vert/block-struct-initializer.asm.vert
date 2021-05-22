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
    float Vert_a [[user(locn0)]];
    float Vert_b [[user(locn1)]];
    float Foo_c [[user(locn2)]];
    float Foo_d [[user(locn3)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    Vert _3 = Vert{ 0.0, 0.0 };
    Foo foo = Foo{ 0.0, 0.0 };
    out.gl_Position = float4(0.0);
    out.Vert_a = _3.a;
    out.Vert_b = _3.b;
    out.Foo_c = foo.c;
    out.Foo_d = foo.d;
    return out;
}

