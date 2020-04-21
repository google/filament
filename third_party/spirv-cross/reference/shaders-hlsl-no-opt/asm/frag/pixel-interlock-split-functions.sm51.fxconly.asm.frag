RasterizerOrderedByteAddressBuffer _7 : register(u1, space0);
RasterizerOrderedByteAddressBuffer _9 : register(u0, space0);

static float4 gl_FragCoord;
struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : SV_Position;
};

void callee2()
{
    int _37 = int(gl_FragCoord.x);
    _7.Store(_37 * 4 + 0, _7.Load(_37 * 4 + 0) + 1u);
}

void callee()
{
    int _45 = int(gl_FragCoord.x);
    _9.Store(_45 * 4 + 0, _9.Load(_45 * 4 + 0) + 1u);
    callee2();
}

void _29()
{
}

void _31()
{
}

void frag_main()
{
    callee();
    _29();
    _31();
}

void main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
}
