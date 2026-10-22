#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct anon_aa
{
    int foo;
};

struct anon_ab
{
    int foo;
};

struct anon_a
{
    anon_aa _aa;
    anon_ab ab;
};

struct anon_ba
{
    int foo;
};

struct anon_bb
{
    int foo;
};

struct anon_b
{
    anon_ba _ba;
    anon_bb bb;
};

struct VertexData
{
    anon_a _a;
    anon_b b;
};

struct anon_ca
{
    int foo;
};

struct anon_c
{
    anon_ca _ca;
};

struct anon_da
{
    int foo;
};

struct anon_d
{
    anon_da da;
};

struct UBO
{
    anon_c _c;
    anon_d d;
};

struct anon_e
{
    int a;
};

struct SSBO
{
    anon_e _m0;
    anon_e _e;
    anon_e f;
};

fragment void main0()
{
}

