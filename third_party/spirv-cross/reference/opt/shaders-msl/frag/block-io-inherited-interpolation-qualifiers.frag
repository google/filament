#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Foo
{
    float2 M;
    float F;
};

struct V
{
    Foo foo;
};

struct main0_out
{
    float Fo [[color(0)]];
};

struct main0_in
{
    float2 m_13_foo_M [[user(locn0), flat]];
    float m_13_foo_F [[user(locn1), flat]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    V _13 = {};
    _13.foo.M = in.m_13_foo_M;
    _13.foo.F = in.m_13_foo_F;
    out.Fo = _13.foo.F;
    return out;
}

