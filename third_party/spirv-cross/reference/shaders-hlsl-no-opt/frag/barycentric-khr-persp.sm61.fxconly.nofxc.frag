static float3 gl_BaryCoordEXT;
static float2 value;
static float2 vUV[3];

struct SPIRV_Cross_Input
{
    nointerpolation float2 vUV : TEXCOORD0;
    float3 gl_BaryCoordEXT : SV_Barycentrics;
};

struct SPIRV_Cross_Output
{
    float2 value : SV_Target0;
};

void frag_main()
{
    value = ((vUV[0] * gl_BaryCoordEXT.x) + (vUV[1] * gl_BaryCoordEXT.y)) + (vUV[2] * gl_BaryCoordEXT.z);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_BaryCoordEXT = stage_input.gl_BaryCoordEXT;
    vUV[0] = GetAttributeAtVertex(stage_input.vUV, 0);
    vUV[1] = GetAttributeAtVertex(stage_input.vUV, 1);
    vUV[2] = GetAttributeAtVertex(stage_input.vUV, 2);
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.value = value;
    return stage_output;
}
