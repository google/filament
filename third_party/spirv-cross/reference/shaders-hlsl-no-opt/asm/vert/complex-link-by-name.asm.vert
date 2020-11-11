struct Struct_vec4
{
    float4 m0;
};

cbuffer UBO : register(b0)
{
    Struct_vec4 ubo_binding_0_m0 : packoffset(c0);
    Struct_vec4 ubo_binding_0_m1 : packoffset(c1);
};


static float4 gl_Position;
static Struct_vec4 output_location_2;
static Struct_vec4 output_location_3;

struct VertexOut
{
    Struct_vec4 m0 : TEXCOORD0;
    Struct_vec4 m1 : TEXCOORD1;
};

static VertexOut output_location_0;

struct SPIRV_Cross_Output
{
    Struct_vec4 output_location_2 : TEXCOORD2;
    Struct_vec4 output_location_3 : TEXCOORD3;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    Struct_vec4 c;
    c.m0 = ubo_binding_0_m0.m0;
    Struct_vec4 b;
    b.m0 = ubo_binding_0_m1.m0;
    gl_Position = c.m0 + b.m0;
    output_location_0.m0 = c;
    output_location_0.m1 = b;
    output_location_2 = c;
    output_location_3 = b;
}

SPIRV_Cross_Output main(out VertexOut stage_outputoutput_location_0)
{
    vert_main();
    stage_outputoutput_location_0 = output_location_0;
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.output_location_2 = output_location_2;
    stage_output.output_location_3 = output_location_3;
    return stage_output;
}
