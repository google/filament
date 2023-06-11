uniform float4 UBO[15];

static float4 gl_Position;
static int2 aIndex;

struct SPIRV_Cross_Input
{
    int2 aIndex : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = UBO[aIndex.x * 5 + aIndex.y * 1 + 0];
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    aIndex = stage_input.aIndex;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
