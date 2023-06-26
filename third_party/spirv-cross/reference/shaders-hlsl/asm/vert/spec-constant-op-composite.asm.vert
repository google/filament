#ifndef SPIRV_CROSS_CONSTANT_ID_201
#define SPIRV_CROSS_CONSTANT_ID_201 -10
#endif
static const int _7 = SPIRV_CROSS_CONSTANT_ID_201;
static const int _20 = (_7 + 2);
#ifndef SPIRV_CROSS_CONSTANT_ID_202
#define SPIRV_CROSS_CONSTANT_ID_202 100u
#endif
static const uint _8 = SPIRV_CROSS_CONSTANT_ID_202;
static const uint _25 = (_8 % 5u);
static const int4 _30 = int4(20, 30, _20, _20);
static const int2 _32 = int2(_30.y, _30.x);
static const int _33 = _30.y;
#ifndef SPIRV_CROSS_CONSTANT_ID_200
#define SPIRV_CROSS_CONSTANT_ID_200 3.141590118408203125f
#endif
static const float _9 = SPIRV_CROSS_CONSTANT_ID_200;

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
