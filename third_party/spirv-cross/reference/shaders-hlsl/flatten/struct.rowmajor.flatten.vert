struct Foo
{
    column_major float3x4 MVP0;
    column_major float3x4 MVP1;
};

uniform float4 UBO[8];

static float4 v0;
static float4 v1;
static float3 V0;
static float3 V1;

struct SPIRV_Cross_Input
{
    float4 v0 : TEXCOORD0;
    float4 v1 : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float3 V0 : TEXCOORD0;
    float3 V1 : TEXCOORD1;
};

void vert_main()
{
    Foo _19 = {transpose(float4x3(UBO[0].xyz, UBO[1].xyz, UBO[2].xyz, UBO[3].xyz)), transpose(float4x3(UBO[4].xyz, UBO[5].xyz, UBO[6].xyz, UBO[7].xyz))};
    Foo _20 = _19;
    Foo f;
    f.MVP0 = _20.MVP0;
    f.MVP1 = _20.MVP1;
    float3 a = mul(f.MVP0, v0);
    float3 b = mul(f.MVP1, v1);
    V0 = a;
    V1 = b;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    v0 = stage_input.v0;
    v1 = stage_input.v1;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.V0 = V0;
    stage_output.V1 = V1;
    return stage_output;
}
