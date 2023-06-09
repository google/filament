struct InstanceData
{
    column_major float4x4 MATRIX_MVP;
    float4 Color;
};

cbuffer gInstanceData : register(b0)
{
    InstanceData gInstanceData_1_data[32] : packoffset(c0);
};


static float4 gl_Position;
static int gl_InstanceIndex;
static float3 PosL;
static float4 _entryPointOutput_Color;

struct SPIRV_Cross_Input
{
    float3 PosL : TEXCOORD0;
    uint gl_InstanceIndex : SV_InstanceID;
};

struct SPIRV_Cross_Output
{
    float4 _entryPointOutput_Color : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(float4(PosL, 1.0f), gInstanceData_1_data[uint(gl_InstanceIndex)].MATRIX_MVP);
    _entryPointOutput_Color = gInstanceData_1_data[uint(gl_InstanceIndex)].Color;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_InstanceIndex = int(stage_input.gl_InstanceIndex);
    PosL = stage_input.PosL;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output._entryPointOutput_Color = _entryPointOutput_Color;
    return stage_output;
}
