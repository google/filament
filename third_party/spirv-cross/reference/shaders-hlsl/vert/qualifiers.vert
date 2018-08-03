static float4 gl_Position;
static float vFlat;
static float vCentroid;
static float vSample;
static float vNoperspective;

struct Block
{
    nointerpolation float vFlat : TEXCOORD4;
    centroid float vCentroid : TEXCOORD5;
    sample float vSample : TEXCOORD6;
    noperspective float vNoperspective : TEXCOORD7;
};

static Block vout;

struct SPIRV_Cross_Output
{
    nointerpolation float vFlat : TEXCOORD0;
    centroid float vCentroid : TEXCOORD1;
    sample float vSample : TEXCOORD2;
    noperspective float vNoperspective : TEXCOORD3;
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

SPIRV_Cross_Output main(out Block stage_outputvout)
{
    vert_main();
    stage_outputvout = vout;
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.vFlat = vFlat;
    stage_output.vCentroid = vCentroid;
    stage_output.vSample = vSample;
    stage_output.vNoperspective = vNoperspective;
    return stage_output;
}
