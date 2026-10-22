struct V2F
{
    float4 Position;
    float4 Color;
};

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

V2F _VS(float3 PosL_1, uint instanceID)
{
    InstanceData instData;
    instData.MATRIX_MVP = gInstanceData_1_data[instanceID].MATRIX_MVP;
    instData.Color = gInstanceData_1_data[instanceID].Color;
    V2F v2f;
    v2f.Position = mul(float4(PosL_1, 1.0f), instData.MATRIX_MVP);
    v2f.Color = instData.Color;
    return v2f;
}

void vert_main()
{
    float3 PosL_1 = PosL;
    uint instanceID = uint(gl_InstanceIndex);
    float3 param = PosL_1;
    uint param_1 = instanceID;
    V2F flattenTemp = _VS(param, param_1);
    gl_Position = flattenTemp.Position;
    _entryPointOutput_Color = flattenTemp.Color;
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
