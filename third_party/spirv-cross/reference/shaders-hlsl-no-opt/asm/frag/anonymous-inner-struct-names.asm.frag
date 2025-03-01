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

struct anon_e
{
    int a;
};

cbuffer UBO : register(b0)
{
    anon_c _18_c : packoffset(c0);
    anon_d _18_d : packoffset(c1);
};

RWByteAddressBuffer _21 : register(u1);

static VertexData _4;

struct SPIRV_Cross_Input
{
    anon_a VertexData__a : TEXCOORD0;
    anon_b VertexData_b : TEXCOORD2;
};

void frag_main()
{
}

void main(SPIRV_Cross_Input stage_input)
{
    _4._a = stage_input.VertexData__a;
    _4.b = stage_input.VertexData_b;
    frag_main();
}
