uniform float4 UBO[12];

static float4 gl_Position;
static float4 aVertex;

struct SPIRV_Cross_Input
{
    float4 aVertex : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(aVertex, float4x4(UBO[0], UBO[1], UBO[2], UBO[3])) + mul(aVertex, transpose(float4x4(UBO[4], UBO[5], UBO[6], UBO[7])));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    aVertex = stage_input.aVertex;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
