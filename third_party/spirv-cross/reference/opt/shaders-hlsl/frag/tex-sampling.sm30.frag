uniform sampler1D tex1d;
uniform sampler2D tex2d;
uniform sampler3D tex3d;
uniform samplerCUBE texCube;
uniform sampler1D tex1dShadow;
uniform sampler2D tex2dShadow;

static float texCoord1d;
static float2 texCoord2d;
static float3 texCoord3d;
static float4 FragColor;
static float4 texCoord4d;

struct SPIRV_Cross_Input
{
    float texCoord1d : TEXCOORD0;
    float2 texCoord2d : TEXCOORD1;
    float3 texCoord3d : TEXCOORD2;
    float4 texCoord4d : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : COLOR0;
};

void frag_main()
{
    float2 _34 = float2(texCoord1d, 2.0f);
    float3 _73 = float3(texCoord2d, 2.0f);
    float4 _112 = float4(texCoord3d, 2.0f);
    float4 _139 = ((((((((((((((((tex1D(tex1d, texCoord1d) + tex1Dlod(tex1d, float4(texCoord1d, 0.0, 0.0, 2.0f))) + tex1Dgrad(tex1d, texCoord1d, 1.0f, 2.0f)) + tex1Dproj(tex1d, float4(_34.x, 0.0, 0.0, _34.y))) + tex1Dbias(tex1d, float4(texCoord1d, 0.0, 0.0, 1.0f))) + tex2D(tex2d, texCoord2d)) + tex2Dlod(tex2d, float4(texCoord2d, 0.0, 2.0f))) + tex2Dgrad(tex2d, texCoord2d, float2(1.0f, 2.0f), float2(3.0f, 4.0f))) + tex2Dproj(tex2d, float4(_73.xy, 0.0, _73.z))) + tex2Dbias(tex2d, float4(texCoord2d, 0.0, 1.0f))) + tex3D(tex3d, texCoord3d)) + tex3Dlod(tex3d, float4(texCoord3d, 2.0f))) + tex3Dgrad(tex3d, texCoord3d, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f))) + tex3Dproj(tex3d, float4(_112.xyz, _112.w))) + tex3Dbias(tex3d, float4(texCoord3d, 1.0f))) + texCUBE(texCube, texCoord3d)) + texCUBElod(texCube, float4(texCoord3d, 2.0f))) + texCUBEbias(texCube, float4(texCoord3d, 1.0f));
    float3 _147 = float3(texCoord1d, 0.0f, 0.0f);
    float4 _171 = float4(texCoord1d, 0.0f, 0.0f, 2.0f);
    _171.y = 2.0f;
    float3 _194 = float3(texCoord2d, 0.0f);
    float4 _219 = float4(texCoord2d, 0.0f, 2.0f);
    _219.z = 2.0f;
    float4 _264 = _139;
    _264.w = (((((((_139.w + tex1Dproj(tex1dShadow, float4(_147.x, 0.0, 0.0f, 1.0)).x) + tex1Dlod(tex1dShadow, float4(_147.x, 0.0, 0.0f, 2.0f)).x) + tex1Dproj(tex1dShadow, float4(_171.x, 0.0, 0.0f, _171.y)).x) + tex1Dbias(tex1dShadow, float4(_147.x, 0.0, 0.0f, 1.0f)).x) + tex2Dproj(tex2dShadow, float4(_194.xy, 0.0f, 1.0)).x) + tex2Dlod(tex2dShadow, float4(_194.xy, 0.0f, 2.0f)).x) + tex2Dproj(tex2dShadow, float4(_219.xy, 0.0f, _219.z)).x) + tex2Dbias(tex2dShadow, float4(_194.xy, 0.0f, 1.0f)).x;
    FragColor = _264;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    texCoord1d = stage_input.texCoord1d;
    texCoord2d = stage_input.texCoord2d;
    texCoord3d = stage_input.texCoord3d;
    texCoord4d = stage_input.texCoord4d;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = float4(FragColor);
    return stage_output;
}
