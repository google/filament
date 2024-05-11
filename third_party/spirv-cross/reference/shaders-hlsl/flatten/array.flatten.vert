uniform float4 UBO[56];

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
    float4 a4 = UBO[23];
    float4 offset = (UBO[50] + UBO[45]) + UBO[54].x.xxxx;
    gl_Position = (mul(aVertex, float4x4(UBO[40], UBO[41], UBO[42], UBO[43])) + UBO[55]) + offset;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    aVertex = stage_input.aVertex;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
