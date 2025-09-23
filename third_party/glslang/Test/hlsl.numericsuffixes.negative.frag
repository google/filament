
struct PS_OUTPUT { float4 color : SV_Target0; };

PS_OUTPUT main()
{
    // Test numeric suffixes
    uint  r01 = 0bERROR321u; // Bad digit
    uint  r02 = 0b11111111111111111111111111111111111111111111111111111111111111111u; // To big
    uint  r03 = 0xTESTu // Bad digit
    uint  r04 = 0xFFFFFFFFFFFFFFFFFFu; // To big
    
    PS_OUTPUT ps_output;
    ps_output.color = r01;
    return ps_output;
}
