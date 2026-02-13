static float4 gl_Position;
static float3 gColor;
static float3 vColorIn[1];
static float4 vPositionIn[1];

struct SPIRV_Cross_Input
{
    float4 vPositionIn : TEXCOORD0;
    float3 vColorIn : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float3 gColor : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void emitPoint(float4 _point, float3 color, inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    gColor = color;
    gl_Position = _point;
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
}

void emitTriangle(float4 center, float3 color, inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    float4 param = center + float4(-0.100000001490116119384765625f, -0.100000001490116119384765625f, 0.0f, 0.0f);
    float3 param_1 = color;
    emitPoint(param, param_1, geometry_stream);
    float4 param_2 = center + float4(0.100000001490116119384765625f, -0.100000001490116119384765625f, 0.0f, 0.0f);
    float3 param_3 = color.zyx;
    emitPoint(param_2, param_3, geometry_stream);
    float4 param_4 = center + float4(0.0f, 0.100000001490116119384765625f, 0.0f, 0.0f);
    float3 param_5 = color.yxz;
    emitPoint(param_4, param_5, geometry_stream);
    geometry_stream.RestartStrip();
}

float4 somePureFunction(float4 data)
{
    return data.wzxy;
}

void functionThatIndirectlyEmits(inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    float4 param = float4(1.0f, 2.0f, 3.0f, 4.0f);
    float3 param_1 = float3(1.0f, 0.0f, 1.0f);
    emitTriangle(param, param_1, geometry_stream);
}

void geom_main(point SPIRV_Cross_Input stage_input[1], inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    float3 color = vColorIn[0];
    float4 param = vPositionIn[0];
    float3 param_1 = color;
    emitTriangle(param, param_1, geometry_stream);
    float4 param_2 = vPositionIn[0];
    float4 param_3 = somePureFunction(param_2);
    float3 param_4 = color;
    emitTriangle(param_3, param_4, geometry_stream);
    functionThatIndirectlyEmits(geometry_stream);
}

[maxvertexcount(9)]
void main(point SPIRV_Cross_Input stage_input[1], inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    for (int i = 0; i < 1; i++)
    {
        vColorIn[i] = stage_input[i].vColorIn;
    }
    for (int i = 0; i < 1; i++)
    {
        vPositionIn[i] = stage_input[i].vPositionIn;
    }
    geom_main(stage_input, geometry_stream);
}
