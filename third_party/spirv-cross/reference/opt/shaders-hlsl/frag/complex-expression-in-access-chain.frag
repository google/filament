RWByteAddressBuffer _34 : register(u0);
Texture2D<int4> Buf : register(t1);
SamplerState _Buf_sampler : register(s1);

static float4 gl_FragCoord;
static int vIn;
static int vIn2;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    nointerpolation int vIn : TEXCOORD0;
    nointerpolation int vIn2 : TEXCOORD1;
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    int _40 = Buf.Load(int3(int2(gl_FragCoord.xy), 0)).x % 16;
    FragColor = (asfloat(_34.Load4(_40 * 16 + 0)) + asfloat(_34.Load4(_40 * 16 + 0))) + asfloat(_34.Load4(((vIn * vIn) + (vIn2 * vIn2)) * 16 + 0));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    vIn = stage_input.vIn;
    vIn2 = stage_input.vIn2;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
