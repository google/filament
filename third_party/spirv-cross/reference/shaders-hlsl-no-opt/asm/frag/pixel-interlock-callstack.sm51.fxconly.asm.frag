RasterizerOrderedByteAddressBuffer _7 : register(u1, space0);
RWByteAddressBuffer _9 : register(u0, space0);

static float4 gl_FragCoord;
struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : SV_Position;
};

void callee2()
{
    int _31 = int(gl_FragCoord.x);
    _7.Store(_31 * 4 + 0, _7.Load(_31 * 4 + 0) + 1u);
}

void callee()
{
    int _39 = int(gl_FragCoord.x);
    _9.Store(_39 * 4 + 0, _9.Load(_39 * 4 + 0) + 1u);
    callee2();
}

void frag_main()
{
    callee();
}

void main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    frag_main();
}
