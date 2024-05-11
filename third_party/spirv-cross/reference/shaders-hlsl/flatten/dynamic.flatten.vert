struct Light
{
    float3 Position;
    float Radius;
    float4 Color;
};

uniform float4 UBO[12];

static float4 gl_Position;
static float4 aVertex;
static float4 vColor;
static float3 aNormal;

struct SPIRV_Cross_Input
{
    float4 aVertex : TEXCOORD0;
    float3 aNormal : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 vColor : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(aVertex, float4x4(UBO[0], UBO[1], UBO[2], UBO[3]));
    vColor = 0.0f.xxxx;
    for (int i = 0; i < 4; i++)
    {
        float3 L = aVertex.xyz - UBO[i * 2 + 4].xyz;
        vColor += ((UBO[i * 2 + 5] * clamp(1.0f - (length(L) / UBO[i * 2 + 4].w), 0.0f, 1.0f)) * dot(aNormal, normalize(L)));
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    aVertex = stage_input.aVertex;
    aNormal = stage_input.aNormal;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.vColor = vColor;
    return stage_output;
}
