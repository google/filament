uniform float4 UBO[8];

static float4 gl_Position;
static float4 oA;
static float4 oB;
static float4 oC;
static float4 oD;
static float4 oE;
static float4 oF;

struct SPIRV_Cross_Output
{
    float4 oA : TEXCOORD0;
    float4 oB : TEXCOORD1;
    float4 oC : TEXCOORD2;
    float4 oD : TEXCOORD3;
    float4 oE : TEXCOORD4;
    float4 oF : TEXCOORD5;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = 0.0f.xxxx;
    oA = UBO[0];
    oB = float4(UBO[1].xy, UBO[1].zw);
    oC = float4(UBO[2].x, UBO[3].xyz);
    oD = float4(UBO[4].xyz, UBO[4].w);
    oE = float4(UBO[5].x, UBO[5].y, UBO[5].z, UBO[5].w);
    oF = float4(UBO[6].x, UBO[6].zw, UBO[7].x);
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
    stage_output.oF = oF;
    return stage_output;
}
