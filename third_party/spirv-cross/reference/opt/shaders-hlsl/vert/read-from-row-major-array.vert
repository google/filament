cbuffer Block : register(b0)
{
    column_major float2x3 _104_var[3][4] : packoffset(c0);
};


static float4 gl_Position;
static float4 a_position;
static float v_vtxResult;

struct SPIRV_Cross_Input
{
    float4 a_position : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float v_vtxResult : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = a_position;
    v_vtxResult = ((float(abs(_104_var[0][0][0].x - 2.0f) < 0.0500000007450580596923828125f) * float(abs(_104_var[0][0][0].y - 6.0f) < 0.0500000007450580596923828125f)) * float(abs(_104_var[0][0][0].z - (-6.0f)) < 0.0500000007450580596923828125f)) * ((float(abs(_104_var[0][0][1].x) < 0.0500000007450580596923828125f) * float(abs(_104_var[0][0][1].y - 5.0f) < 0.0500000007450580596923828125f)) * float(abs(_104_var[0][0][1].z - 5.0f) < 0.0500000007450580596923828125f));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    a_position = stage_input.a_position;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.v_vtxResult = v_vtxResult;
    return stage_output;
}
