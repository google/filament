uniform float4 UBO[14];

static float4 gl_Position;
static float4 oA;
static float4 oB;
static float4 oC;
static float4 oD;
static float4 oE;

struct SPIRV_Cross_Output
{
    float4 oA : TEXCOORD0;
    float4 oB : TEXCOORD1;
    float4 oC : TEXCOORD2;
    float4 oD : TEXCOORD3;
    float4 oE : TEXCOORD4;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = 0.0f.xxxx;
    oA = UBO[1];
    oB = float4(UBO[4].y, UBO[5].y, UBO[6].y, UBO[7].y);
    oC = UBO[9];
    oD = float4(UBO[10].x, UBO[11].x, UBO[12].x, UBO[13].x);
    oE = float4(UBO[1].z, UBO[6].y, UBO[9].z, UBO[12].y);
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.oA = oA;
    stage_output.oB = oB;
    stage_output.oC = oC;
    stage_output.oD = oD;
    stage_output.oE = oE;
    return stage_output;
}
