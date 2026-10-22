static int oA;
static int A;
static uint B;
static uint oB;
static int C;
static uint D;

struct SPIRV_Cross_Input
{
    nointerpolation int A : TEXCOORD0;
    nointerpolation uint B : TEXCOORD1;
    nointerpolation int C : TEXCOORD2;
    nointerpolation uint D : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    int oA : SV_Target0;
    uint oB : SV_Target1;
};

void frag_main()
{
    oB = uint(A - int(B) * (A / int(B)));
    oB = uint(A - C * (A / C));
    oB = uint(int(B) - int(D) * (int(B) / int(D)));
    oB = uint(int(B) - A * (int(B) / A));
    oA = A - int(B) * (A / int(B));
    oA = A - C * (A / C);
    oA = int(B) - int(D) * (int(B) / int(D));
    oA = int(B) - A * (int(B) / A);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    A = stage_input.A;
    B = stage_input.B;
    C = stage_input.C;
    D = stage_input.D;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.oA = oA;
    stage_output.oB = oB;
    return stage_output;
}
