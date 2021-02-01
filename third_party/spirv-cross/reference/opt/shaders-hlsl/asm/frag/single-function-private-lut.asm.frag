struct myType
{
    float data;
};

static const myType _18 = { 0.0f };
static const myType _20 = { 1.0f };
static const myType _21[5] = { { 0.0f }, { 1.0f }, { 0.0f }, { 1.0f }, { 0.0f } };

static float4 gl_FragCoord;
static float4 o_color;

struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    float4 o_color : SV_Target0;
};

float mod(float x, float y)
{
    return x - y * floor(x / y);
}

float2 mod(float2 x, float2 y)
{
    return x - y * floor(x / y);
}

float3 mod(float3 x, float3 y)
{
    return x - y * floor(x / y);
}

float4 mod(float4 x, float4 y)
{
    return x - y * floor(x / y);
}

void frag_main()
{
    if (_21[int(mod(gl_FragCoord.x, 4.0f))].data > 0.0f)
    {
        o_color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    }
    else
    {
        o_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.o_color = o_color;
    return stage_output;
}
