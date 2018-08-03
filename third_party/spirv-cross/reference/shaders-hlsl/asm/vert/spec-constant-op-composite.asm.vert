static const int _7 = -10;
static const uint _8 = 100u;
static const float _9 = 3.141590118408203125f;
static const int _20 = (_7 + 2);
static const uint _25 = (_8 % 5u);
static const int4 _30 = int4(20, 30, _20, _20);
static const int2 _32 = int2(_30.y, _30.x);
static const int _33 = _30.y;

static float4 gl_Position;
static int _4;

struct SPIRV_Cross_Output
{
    nointerpolation int _4 : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    float4 pos = 0.0f.xxxx;
    pos.y += float(_20);
    pos.z += float(_25);
    pos += float4(_30);
    float2 _56 = pos.xy + float2(_32);
    pos = float4(_56.x, _56.y, pos.z, pos.w);
    gl_Position = pos;
    _4 = _33;
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output._4 = _4;
    return stage_output;
}
