
struct PS_OUTPUT { float4 color : SV_Target0; };

#define BIN_UINT 0b00001u
#define BIN_INT  0b00011

PS_OUTPUT main()
{
    // Test numeric suffixes
    float r00 = 1.0f;    // float
    uint  r01 = 1u;      // lower uint
    uint  r02 = 2U;      // upper uint
    uint  r03 = 0xabcu;  // lower hex uint
    uint  r04 = 0XABCU;  // upper hex uint
    int   r05 = 5l;      // lower long int
    int   r06 = 6L;      // upper long int
    int   r07 = 071;     // octal
    uint  r08 = 072u;    // unsigned octal
    float r09 = 1.h;     // half
    float r10 = 1.H;     // half
    float r11 = 1.1h;    // half
    float r12 = 1.1H;    // half
    uint  r13 = 0b00001u;// lower binary uint
    uint  r14 = 0B00010U;// upper binary uint
    int   r15 = 0b00011; // lower binary int
    int   r16 = 0B00100; // upper binary int
    uint  r17 = BIN_UINT;// lower binart define uint
    int   r18 = BIN_INT; // lower binart define int

    PS_OUTPUT ps_output;
    ps_output.color = r07; // gets 71 octal = 57 decimal
    return ps_output;
}
