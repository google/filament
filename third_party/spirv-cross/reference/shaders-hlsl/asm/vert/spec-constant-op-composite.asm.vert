#ifndef SPIRV_CROSS_CONSTANT_ID_201
#define SPIRV_CROSS_CONSTANT_ID_201 -10
#endif
static const int _13 = SPIRV_CROSS_CONSTANT_ID_201;
static const int _15 = (_13 + 2);
#ifndef SPIRV_CROSS_CONSTANT_ID_202
#define SPIRV_CROSS_CONSTANT_ID_202 100u
#endif
static const uint _24 = SPIRV_CROSS_CONSTANT_ID_202;
static const uint _26 = (_24 % 5u);
static const int4 _36 = int4(20, 30, _15, _15);
static const int2 _41 = int2(_36.y, _36.x);
static const int _60 = _36.y;
#ifndef SPIRV_CROSS_CONSTANT_ID_200
#define SPIRV_CROSS_CONSTANT_ID_200 3.141590118408203125f
#endif
static const float _57 = SPIRV_CROSS_CONSTANT_ID_200;

static float4 gl_Position;
static int _58;

struct SPIRV_Cross_Output
{
    nointerpolation int _58 : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    float4 pos = 0.0f.xxxx;
    pos.y += float(_15);
    pos.z += float(_26);
    pos += float4(_36);
    float2 _46 = pos.xy + float2(_41);
    pos = float4(_46.x, _46.y, pos.z, pos.w);
    gl_Position = pos;
    _58 = _60;
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output._58 = _58;
    return stage_output;
}
