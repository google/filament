#version 310 es
precision mediump float;
precision highp int;

layout(binding = 4, std140) uniform GlobalPSData
{
    vec4 g_CamPos;
    vec4 g_SunDir;
    vec4 g_SunColor;
    vec4 g_ResolutionParams;
    vec4 g_TimeParams;
    vec4 g_FogColor_Distance;
} _101;

layout(binding = 2) uniform mediump sampler2D TexNormalmap;

layout(location = 3) out vec4 LightingOut;
layout(location = 2) out vec4 NormalOut;
layout(location = 1) out vec4 SpecularOut;
layout(location = 0) out vec4 AlbedoOut;
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec3 EyeVec;

float saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}

void Resolve(vec3 Albedo, vec3 Normal, float Roughness, float Metallic)
{
    LightingOut = vec4(0.0);
    NormalOut = vec4((Normal * 0.5) + vec3(0.5), 0.0);
    SpecularOut = vec4(Roughness, Metallic, 0.0, 0.0);
    AlbedoOut = vec4(Albedo, 1.0);
}

void main()
{
    vec3 Normal = (texture(TexNormalmap, TexCoord).xyz * 2.0) - vec3(1.0);
    Normal = normalize(Normal);
    highp float param = length(EyeVec) / 1000.0;
    vec2 scatter_uv;
    scatter_uv.x = saturate(param);
    vec3 nEye = normalize(EyeVec);
    scatter_uv.y = 0.0;
    vec3 Color = vec3(0.100000001490116119384765625, 0.300000011920928955078125, 0.100000001490116119384765625);
    vec3 grass = vec3(0.100000001490116119384765625, 0.300000011920928955078125, 0.100000001490116119384765625);
    vec3 dirt = vec3(0.100000001490116119384765625);
    vec3 snow = vec3(0.800000011920928955078125);
    float grass_snow = smoothstep(0.0, 0.1500000059604644775390625, (_101.g_CamPos.y + EyeVec.y) / 200.0);
    vec3 base = mix(grass, snow, vec3(grass_snow));
    float edge = smoothstep(0.699999988079071044921875, 0.75, Normal.y);
    Color = mix(dirt, base, vec3(edge));
    Color *= Color;
    float Roughness = 1.0 - (edge * grass_snow);
    highp vec3 param_1 = Color;
    highp vec3 param_2 = Normal;
    highp float param_3 = Roughness;
    highp float param_4 = 0.0;
    Resolve(param_1, param_2, param_3, param_4);
}

