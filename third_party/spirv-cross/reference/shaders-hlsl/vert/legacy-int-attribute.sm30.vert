uniform float4 gl_HalfPixel;

static float4 gl_Position;
static int4 attr_int4;
static int attr_int1;

struct SPIRV_Cross_Input
{
    float4 attr_int4 : TEXCOORD0;
    float attr_int1 : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : POSITION;
};

void vert_main()
{
    gl_Position.x = float(attr_int4[attr_int1]);
    gl_Position.y = 0.0f;
    gl_Position.z = 0.0f;
    gl_Position.w = 0.0f;
    gl_Position.x = gl_Position.x - gl_HalfPixel.x * gl_Position.w;
    gl_Position.y = gl_Position.y + gl_HalfPixel.y * gl_Position.w;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    attr_int4 = stage_input.attr_int4;
    attr_int1 = stage_input.attr_int1;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
