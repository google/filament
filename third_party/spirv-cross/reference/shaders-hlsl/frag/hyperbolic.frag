static float4 result;
static float scalar;
static float3 _vector;

struct SPIRV_Cross_Input
{
    float scalar : TEXCOORD0;
    float3 _vector : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 result : SV_Target0;
};

void frag_main()
{
    result = 1.0f.xxxx;
    result.w *= sinh(scalar);
    result.w *= cosh(scalar);
    result.w *= tanh(scalar);
    result.w *= log(scalar + sqrt(scalar * scalar + 1.0f));
    result.w *= log(scalar + sqrt(scalar * scalar - 1.0f));
    result.w *= (log((1.0f + scalar) / (1.0f - scalar)) * 0.5f);
    float4 _58 = result;
    float3 _60 = _58.xyz * sinh(_vector);
    result.x = _60.x;
    result.y = _60.y;
    result.z = _60.z;
    float4 _72 = result;
    float3 _74 = _72.xyz * cosh(_vector);
    result.x = _74.x;
    result.y = _74.y;
    result.z = _74.z;
    float4 _83 = result;
    float3 _85 = _83.xyz * tanh(_vector);
    result.x = _85.x;
    result.y = _85.y;
    result.z = _85.z;
    float4 _94 = result;
    float3 _96 = _94.xyz * log(_vector + sqrt(_vector * _vector + 1.0f));
    result.x = _96.x;
    result.y = _96.y;
    result.z = _96.z;
    float4 _105 = result;
    float3 _107 = _105.xyz * log(_vector + sqrt(_vector * _vector - 1.0f));
    result.x = _107.x;
    result.y = _107.y;
    result.z = _107.z;
    float4 _116 = result;
    float3 _118 = _116.xyz * (log((1.0f + _vector) / (1.0f - _vector)) * 0.5f);
    result.x = _118.x;
    result.y = _118.y;
    result.z = _118.z;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    scalar = stage_input.scalar;
    _vector = stage_input._vector;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.result = result;
    return stage_output;
}
