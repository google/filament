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

void main()
{
    vec3 _68 = normalize((texture(TexNormalmap, TexCoord).xyz * 2.0) - vec3(1.0));
    float _113 = smoothstep(0.0, 0.1500000059604644775390625, (_101.g_CamPos.y + EyeVec.y) * 0.004999999888241291046142578125);
    float _125 = smoothstep(0.699999988079071044921875, 0.75, _68.y);
    vec3 _130 = mix(vec3(0.100000001490116119384765625), mix(vec3(0.100000001490116119384765625, 0.300000011920928955078125, 0.100000001490116119384765625), vec3(0.800000011920928955078125), vec3(_113)), vec3(_125));
    LightingOut = vec4(0.0);
    NormalOut = vec4((_68 * 0.5) + vec3(0.5), 0.0);
    SpecularOut = vec4(1.0 - (_125 * _113), 0.0, 0.0, 0.0);
    AlbedoOut = vec4(_130 * _130, 1.0);
}

