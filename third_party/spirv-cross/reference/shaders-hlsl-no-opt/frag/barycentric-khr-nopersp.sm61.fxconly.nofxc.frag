static float3 gl_BaryCoordNoPerspEXT;
static float2 value;
static float2 vUV2[3];

struct SPIRV_Cross_Input
{
    nointerpolation float2 vUV2 : TEXCOORD1;
    noperspective float3 gl_BaryCoordNoPerspEXT : SV_Barycentrics;
};

struct SPIRV_Cross_Output
{
    float2 value : SV_Target0;
};

void frag_main()
{
    value = ((vUV2[0] * gl_BaryCoordNoPerspEXT.x) + (vUV2[1] * gl_BaryCoordNoPerspEXT.y)) + (vUV2[2] * gl_BaryCoordNoPerspEXT.z);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_BaryCoordNoPerspEXT = stage_input.gl_BaryCoordNoPerspEXT;
    vUV2[0] = GetAttributeAtVertex(stage_input.vUV2, 0);
    vUV2[1] = GetAttributeAtVertex(stage_input.vUV2, 1);
    vUV2[2] = GetAttributeAtVertex(stage_input.vUV2, 2);
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.value = value;
    return stage_output;
}
