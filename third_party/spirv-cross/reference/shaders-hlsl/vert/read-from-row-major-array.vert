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

float compare_float(float a, float b)
{
    return float(abs(a - b) < 0.0500000007450580596923828125f);
}

float compare_vec3(float3 a, float3 b)
{
    float param = a.x;
    float param_1 = b.x;
    float param_2 = a.y;
    float param_3 = b.y;
    float param_4 = a.z;
    float param_5 = b.z;
    return (compare_float(param, param_1) * compare_float(param_2, param_3)) * compare_float(param_4, param_5);
}

float compare_mat2x3(float2x3 a, float2x3 b)
{
    float3 param = a[0];
    float3 param_1 = b[0];
    float3 param_2 = a[1];
    float3 param_3 = b[1];
    return compare_vec3(param, param_1) * compare_vec3(param_2, param_3);
}

void vert_main()
{
    gl_Position = a_position;
    float result = 1.0f;
    float2x3 param = _104_var[0][0];
    float2x3 param_1 = float2x3(float3(2.0f, 6.0f, -6.0f), float3(0.0f, 5.0f, 5.0f));
    result *= compare_mat2x3(param, param_1);
    v_vtxResult = result;
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
