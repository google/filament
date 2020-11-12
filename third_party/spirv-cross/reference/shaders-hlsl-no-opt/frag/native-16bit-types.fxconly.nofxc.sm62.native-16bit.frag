RWByteAddressBuffer _62 : register(u0, space0);

static float4 gl_FragCoord;
static half4 Output;
static half4 Input;
static int16_t4 OutputI;
static int16_t4 InputI;
static uint16_t4 OutputU;
static uint16_t4 InputU;

struct SPIRV_Cross_Input
{
    half4 Input : TEXCOORD0;
    nointerpolation int16_t4 InputI : TEXCOORD1;
    nointerpolation uint16_t4 InputU : TEXCOORD2;
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    half4 Output : SV_Target0;
    int16_t4 OutputI : SV_Target1;
    uint16_t4 OutputU : SV_Target2;
};

void frag_main()
{
    int index = int(gl_FragCoord.x);
    Output = Input + half(20.0).xxxx;
    OutputI = InputI + int16_t4(int16_t(-40), int16_t(-40), int16_t(-40), int16_t(-40));
    OutputU = InputU + uint16_t4(20u, 20u, 20u, 20u);
    Output += _62.Load<half>(index * 2 + 0).xxxx;
    OutputI += _62.Load<int16_t>(index * 2 + 8).xxxx;
    OutputU += _62.Load<uint16_t>(index * 2 + 16).xxxx;
    Output += _62.Load<half4>(index * 8 + 24);
    OutputI += _62.Load<int16_t4>(index * 8 + 56);
    OutputU += _62.Load<uint16_t4>(index * 8 + 88);
    Output += _62.Load<half3>(index * 16 + 128).xyzz;
    Output += half3(_62.Load<half>(index * 12 + 186), _62.Load<half>(index * 12 + 190), _62.Load<half>(index * 12 + 194)).xyzz;
    half2x3 _128 = half2x3(_62.Load<half3>(index * 16 + 120), _62.Load<half3>(index * 16 + 128));
    half2x3 m0 = _128;
    half2x3 _132 = half2x3(_62.Load<half>(index * 12 + 184), _62.Load<half>(index * 12 + 188), _62.Load<half>(index * 12 + 192), _62.Load<half>(index * 12 + 186), _62.Load<half>(index * 12 + 190), _62.Load<half>(index * 12 + 194));
    half2x3 m1 = _132;
    _62.Store<half>(index * 2 + 0, Output.x);
    _62.Store<int16_t>(index * 2 + 8, OutputI.y);
    _62.Store<uint16_t>(index * 2 + 16, OutputU.z);
    _62.Store<half4>(index * 8 + 24, Output);
    _62.Store<int16_t4>(index * 8 + 56, OutputI);
    _62.Store<uint16_t4>(index * 8 + 88, OutputU);
    _62.Store<half3>(index * 16 + 128, Output.xyz);
    _62.Store<half>(index * 12 + 186, Output.x);
    _62.Store<half>(index * 12 + 190, Output.xyz.y);
    _62.Store<half>(index * 12 + 194, Output.xyz.z);
    half2x3 _182 = half2x3(half3(Output.xyz), half3(Output.wzy));
    _62.Store<half3>(index * 16 + 120, _182[0]);
    _62.Store<half3>(index * 16 + 128, _182[1]);
    half2x3 _197 = half2x3(half3(Output.xyz), half3(Output.wzy));
    _62.Store<half>(index * 12 + 184, _197[0].x);
    _62.Store<half>(index * 12 + 186, _197[1].x);
    _62.Store<half>(index * 12 + 188, _197[0].y);
    _62.Store<half>(index * 12 + 190, _197[1].y);
    _62.Store<half>(index * 12 + 192, _197[0].z);
    _62.Store<half>(index * 12 + 194, _197[1].z);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    Input = stage_input.Input;
    InputI = stage_input.InputI;
    InputU = stage_input.InputU;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.Output = Output;
    stage_output.OutputI = OutputI;
    stage_output.OutputU = OutputU;
    return stage_output;
}
