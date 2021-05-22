static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

uint64_t spvPackUint2x32(uint2 value)
{
    return (uint64_t(value.y) << 32) | uint64_t(value.x);
}

uint2 spvUnpackUint2x32(uint64_t value)
{
    uint2 Unpacked;
    Unpacked.x = uint(value & 0xffffffff);
    Unpacked.y = uint(value >> 32);
    return Unpacked;
}

void frag_main()
{
    uint2 unpacked = spvUnpackUint2x32(spvPackUint2x32(uint2(18u, 52u)));
    FragColor = float4(float(unpacked.x), float(unpacked.y), 1.0f, 1.0f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
