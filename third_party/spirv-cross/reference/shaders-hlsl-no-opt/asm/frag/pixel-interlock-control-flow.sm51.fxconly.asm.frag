RasterizerOrderedByteAddressBuffer _7 : register(u1, space0);
RWByteAddressBuffer _13 : register(u2, space0);
RasterizerOrderedByteAddressBuffer _9 : register(u0, space0);

static float4 gl_FragCoord;
struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : SV_Position;
};

void callee2()
{
    int _44 = int(gl_FragCoord.x);
    _7.Store(_44 * 4 + 0, _7.Load(_44 * 4 + 0) + 1u);
}

void callee()
{
    int _52 = int(gl_FragCoord.x);
    _9.Store(_52 * 4 + 0, _9.Load(_52 * 4 + 0) + 1u);
    callee2();
    if (true)
    {
    }
}

void _35()
{
    _13.Store(int(gl_FragCoord.x) * 4 + 0, 4u);
}

void frag_main()
{
    callee();
    _35();
}

void main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    frag_main();
}
