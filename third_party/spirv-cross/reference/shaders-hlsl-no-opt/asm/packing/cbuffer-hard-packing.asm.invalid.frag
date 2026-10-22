struct StraddleResolve
{
    float3 A;
    float3 B;
    float3 C;
    float3 D;
};

struct Test1
{
    StraddleResolve a;
    float b;
};

struct Test2
{
    StraddleResolve a[2];
    float b;
    StraddleResolve c[3][2];
    float d;
};

struct MatrixStraddle2x3c
{
    column_major float2x3 m;
};

struct MatrixStraddle2x3r
{
    row_major float2x3 m;
};

struct MatrixStraddle3x2c
{
    column_major float3x2 m;
};

struct MatrixStraddle3x2r
{
    row_major float3x2 m;
};

struct Test3
{
    MatrixStraddle2x3c c23;
    float dummy0;
    MatrixStraddle2x3r r23;
    float dummy1;
    MatrixStraddle3x2c c32;
    float dummy2;
    MatrixStraddle3x2r r32;
    float dummy3;
};

struct Test4
{
    MatrixStraddle2x3c c23[2][3];
    float dummy0;
    MatrixStraddle2x3r r23[2][3];
    float dummy1;
};

cbuffer type_Test1Cbuf : register(b0)
{
    Test1 Test1Cbuf_test1 : packoffset(c0);
};

cbuffer type_Test2Cbuf : register(b1)
{
    Test2 Test2Cbuf_test2 : packoffset(c0);
};

cbuffer type_Test3Cbuf : register(b2)
{
    Test3 Test3Cbuf_test3 : packoffset(c0);
};

cbuffer type_Test4Cbuf : register(b3)
{
    Test4 Test4Cbuf_test4 : packoffset(c0);
};


static float4 in_var_COLOR;
static float4 out_var_SV_TARGET;

struct SPIRV_Cross_Input
{
    float4 in_var_COLOR : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 out_var_SV_TARGET : SV_Target0;
};

void frag_main()
{
    out_var_SV_TARGET = (((in_var_COLOR + Test1Cbuf_test1.b.xxxx) + Test2Cbuf_test2.b.xxxx) + Test3Cbuf_test3.dummy0.xxxx) + Test4Cbuf_test4.dummy0.xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    in_var_COLOR = stage_input.in_var_COLOR;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.out_var_SV_TARGET = out_var_SV_TARGET;
    return stage_output;
}
