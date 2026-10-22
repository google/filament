struct Block
{
    float vFlat;
    float vCentroid;
    float vSample;
    float vNoperspective;
};

static float4 gl_Position;
static float vFlat;
static float vCentroid;
static float vSample;
static float vNoperspective;
static Block vout;

struct SPIRV_Cross_Output
{
    nointerpolation float vFlat : TEXCOORD0;
    centroid float vCentroid : TEXCOORD1;
    sample float vSample : TEXCOORD2;
    noperspective float vNoperspective : TEXCOORD3;
    nointerpolation float Block_vFlat : TEXCOORD4;
    centroid float Block_vCentroid : TEXCOORD5;
    sample float Block_vSample : TEXCOORD6;
    noperspective float Block_vNoperspective : TEXCOORD7;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = 1.0f.xxxx;
    vFlat = 0.0f;
    vCentroid = 1.0f;
    vSample = 2.0f;
    vNoperspective = 3.0f;
    vout.vFlat = 0.0f;
    vout.vCentroid = 1.0f;
    vout.vSample = 2.0f;
    vout.vNoperspective = 3.0f;
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.vFlat = vFlat;
    stage_output.vCentroid = vCentroid;
    stage_output.vSample = vSample;
    stage_output.vNoperspective = vNoperspective;
    stage_output.Block_vFlat = vout.vFlat;
    stage_output.Block_vCentroid = vout.vCentroid;
    stage_output.Block_vSample = vout.vSample;
    stage_output.Block_vNoperspective = vout.vNoperspective;
    return stage_output;
}
