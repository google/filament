struct Foo
{
    float2 a;
    float2 b;
};

static float3 gl_BaryCoordEXT;
static float3 gl_BaryCoordNoPerspEXT;
static float2 value;
static float2 vUV[3];
static float2 vUV2[3];
static Foo foo[3];

struct SPIRV_Cross_Input
{
    nointerpolation float2 vUV : TEXCOORD0;
    nointerpolation float2 vUV2 : TEXCOORD1;
    nointerpolation float2 Foo_a : TEXCOORD2;
    nointerpolation float2 Foo_b : TEXCOORD3;
    float3 gl_BaryCoordEXT : SV_Barycentrics0;
    noperspective float3 gl_BaryCoordNoPerspEXT : SV_Barycentrics1;
};

struct SPIRV_Cross_Output
{
    float2 value : SV_Target0;
};

void frag_main()
{
    value = ((vUV[0] * gl_BaryCoordEXT.x) + (vUV[1] * gl_BaryCoordEXT.y)) + (vUV[2] * gl_BaryCoordEXT.z);
    value += (((vUV2[0] * gl_BaryCoordNoPerspEXT.x) + (vUV2[1] * gl_BaryCoordNoPerspEXT.y)) + (vUV2[2] * gl_BaryCoordNoPerspEXT.z));
    value += (foo[0].a * gl_BaryCoordEXT.x);
    value += (foo[0].b * gl_BaryCoordEXT.y);
    value += (foo[1].a * gl_BaryCoordEXT.z);
    value += (foo[1].b * gl_BaryCoordEXT.x);
    value += (foo[2].a * gl_BaryCoordEXT.y);
    value += (foo[2].b * gl_BaryCoordEXT.z);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_BaryCoordEXT = stage_input.gl_BaryCoordEXT;
    gl_BaryCoordNoPerspEXT = stage_input.gl_BaryCoordNoPerspEXT;
    vUV[0] = GetAttributeAtVertex(stage_input.vUV, 0);
    vUV[1] = GetAttributeAtVertex(stage_input.vUV, 1);
    vUV[2] = GetAttributeAtVertex(stage_input.vUV, 2);
    vUV2[0] = GetAttributeAtVertex(stage_input.vUV2, 0);
    vUV2[1] = GetAttributeAtVertex(stage_input.vUV2, 1);
    vUV2[2] = GetAttributeAtVertex(stage_input.vUV2, 2);
    foo[0].a = GetAttributeAtVertex(stage_input.Foo_a, 0);
    foo[1].a = GetAttributeAtVertex(stage_input.Foo_a, 1);
    foo[2].a = GetAttributeAtVertex(stage_input.Foo_a, 2);
    foo[0].b = GetAttributeAtVertex(stage_input.Foo_b, 0);
    foo[1].b = GetAttributeAtVertex(stage_input.Foo_b, 1);
    foo[2].b = GetAttributeAtVertex(stage_input.Foo_b, 2);
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.value = value;
    return stage_output;
}
