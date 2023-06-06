static const float4 _68[4] = { 0.0f.xxxx, 1.0f.xxxx, 2.0f.xxxx, 3.0f.xxxx };

static float4 gl_Position;
static int Index1;
static int Index2;

struct SPIRV_Cross_Input
{
    int Index1 : TEXCOORD0;
    int Index2 : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

float4 consume_constant_arrays2(float4 positions[4], float4 positions2[4])
{
    float4 indexable[4] = positions;
    float4 indexable_1[4] = positions2;
    return indexable[Index1] + indexable_1[Index2];
}

float4 consume_constant_arrays(float4 positions[4], float4 positions2[4])
{
    return consume_constant_arrays2(positions, positions2);
}

void vert_main()
{
    float4 LUT2[4];
    LUT2[0] = 10.0f.xxxx;
    LUT2[1] = 11.0f.xxxx;
    LUT2[2] = 12.0f.xxxx;
    LUT2[3] = 13.0f.xxxx;
    gl_Position = consume_constant_arrays(_68, LUT2);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    Index1 = stage_input.Index1;
    Index2 = stage_input.Index2;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
