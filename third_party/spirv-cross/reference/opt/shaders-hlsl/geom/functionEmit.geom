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

void geom_main(point SPIRV_Cross_Input stage_input[1], inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    gColor = vColorIn[0];
    gl_Position = vPositionIn[0] + float4(-0.100000001490116119384765625f, -0.100000001490116119384765625f, 0.0f, 0.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    gColor = vColorIn[0].zyx;
    gl_Position = vPositionIn[0] + float4(0.100000001490116119384765625f, -0.100000001490116119384765625f, 0.0f, 0.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    gColor = vColorIn[0].yxz;
    gl_Position = vPositionIn[0] + float4(0.0f, 0.100000001490116119384765625f, 0.0f, 0.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    geometry_stream.RestartStrip();
    gColor = vColorIn[0];
    gl_Position = vPositionIn[0].wzxy + float4(-0.100000001490116119384765625f, -0.100000001490116119384765625f, 0.0f, 0.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    gColor = vColorIn[0].zyx;
    gl_Position = vPositionIn[0].wzxy + float4(0.100000001490116119384765625f, -0.100000001490116119384765625f, 0.0f, 0.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    gColor = vColorIn[0].yxz;
    gl_Position = vPositionIn[0].wzxy + float4(0.0f, 0.100000001490116119384765625f, 0.0f, 0.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    geometry_stream.RestartStrip();
    gColor = float3(1.0f, 0.0f, 1.0f);
    gl_Position = float4(0.89999997615814208984375f, 1.89999997615814208984375f, 3.0f, 4.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    gColor = float3(1.0f, 0.0f, 1.0f);
    gl_Position = float4(1.10000002384185791015625f, 1.89999997615814208984375f, 3.0f, 4.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    gColor = float3(0.0f, 1.0f, 1.0f);
    gl_Position = float4(1.0f, 2.099999904632568359375f, 3.0f, 4.0f);
    {
        SPIRV_Cross_Output stage_output;
        stage_output.gl_Position = gl_Position;
        stage_output.gColor = gColor;
        geometry_stream.Append(stage_output);
    }
    geometry_stream.RestartStrip();
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
