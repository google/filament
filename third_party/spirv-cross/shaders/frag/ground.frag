#version 310 es
precision mediump float;

#define DEBUG_NONE      0
#define DEBUG_DIFFUSE   1
#define DEBUG_SPECULAR  2
#define DEBUG_LIGHTING  3
#define DEBUG_FOG       4
#define DEBUG           DEBUG_NONE

#define FORWARD   0
#define DEFERRED  1
#define DEFERRED_VTEX 2

float saturate(float x) { return clamp(x, 0.0, 1.0); }

layout(std140, binding = 4) uniform GlobalPSData
{
    vec4 g_CamPos;
    vec4 g_SunDir;
    vec4 g_SunColor;
    vec4 g_ResolutionParams;
    vec4 g_TimeParams;
    vec4 g_FogColor_Distance;
};

vec4 ComputeFogFactor(vec3 WorldPos)
{
    vec4 FogData;
    vec3 vEye = WorldPos - g_CamPos.xyz;
    vec3 nEye = normalize(vEye);
    FogData.w = exp(-dot(vEye, vEye) * g_FogColor_Distance.w * 0.75);

    float fog_sun_factor = pow(saturate(dot(nEye, g_SunDir.xyz)), 8.0);
    FogData.xyz = mix(vec3(1.0, 1.0, 1.0), vec3(0.6, 0.6, 0.9), nEye.y * 0.5 + 0.5);
    FogData.xyz = mix(FogData.xyz, vec3(0.95, 0.87, 0.78), fog_sun_factor);
    return FogData;
}

void ApplyFog(inout vec3 Color, vec4 FogData)
{
    Color = mix(FogData.xyz, Color, FogData.w);
}

void ApplyLighting(inout mediump vec3 Color, mediump float DiffuseFactor)
{
    mediump vec3 DiffuseLight = g_SunColor.xyz * DiffuseFactor;
    mediump vec3 AmbientLight = vec3(0.2, 0.35, 0.55) * 0.5;
    mediump vec3 Lighting = DiffuseLight + AmbientLight;
#if DEBUG == DEBUG_LIGHTING
    Color = Lighting;
#else
    Color *= Lighting;
#endif
}

#define SPECULAR 0
#define GLOSSMAP 0

void ApplySpecular(inout mediump vec3 Color, mediump vec3 EyeVec, mediump vec3 Normal, mediump vec3 SpecularColor, mediump float Shininess, mediump float FresnelAmount)
{
    mediump vec3 HalfAngle = normalize(-EyeVec + g_SunDir.xyz);

    mediump float v_dot_h = saturate(dot(HalfAngle, -EyeVec));
    mediump float n_dot_l = saturate(dot(Normal, g_SunDir.xyz));
    mediump float n_dot_h = saturate(dot(Normal, HalfAngle));
    mediump float n_dot_v = saturate(dot(-EyeVec, Normal));
    mediump float h_dot_l = saturate(dot(g_SunDir.xyz, HalfAngle));

    const mediump float roughness_value = 0.25;

    mediump float r_sq = roughness_value * roughness_value;
    mediump float n_dot_h_sq = n_dot_h * n_dot_h;
    mediump float roughness_a = 1.0 / (4.0 * r_sq * n_dot_h_sq * n_dot_h_sq);
    mediump float roughness_b = n_dot_h_sq - 1.0;
    mediump float roughness_c = r_sq * n_dot_h_sq;
    mediump float roughness = saturate(roughness_a * exp(roughness_b / roughness_c));

    FresnelAmount = 0.5;
    mediump float fresnel_term = pow(1.0 - n_dot_v, 5.0) * (1.0 - FresnelAmount) + FresnelAmount;

    mediump float geo_numerator = 2.0 * n_dot_h;
    mediump float geo_denominator = 1.0 / v_dot_h;
    mediump float geo_term = min(1.0, min(n_dot_v, n_dot_l) * geo_numerator * geo_denominator);

#if SPECULAR || GLOSSMAP
    Color += SpecularColor * g_SunColor.xyz * fresnel_term * roughness * n_dot_l * geo_term / (n_dot_v * n_dot_l + 0.0001);
#endif

    //Color = vec3(0.025 * 1.0 / (n_dot_v * n_dot_l));
}
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec3 EyeVec;

layout(binding = 2) uniform sampler2D TexNormalmap;
//layout(binding = 3) uniform sampler2D TexScatteringLUT;

#define DIFFUSE_ONLY 0
#define GLOBAL_RENDERER DEFERRED
#define OUTPUT_FEEDBACK_TEXTURE 0

#if DIFFUSE_ONLY
layout(location = 0) out vec4 ColorOut;
layout(location = 1) out vec4 NormalOut;
#else
layout(location = 0) out vec4 AlbedoOut;
layout(location = 1) out vec4 SpecularOut;
layout(location = 2) out vec4 NormalOut;
layout(location = 3) out vec4 LightingOut;
#endif

void Resolve(vec3 Albedo, vec3 Normal, float Roughness, float Metallic)
{
#if (GLOBAL_RENDERER == FORWARD) || OUTPUT_FEEDBACK_TEXTURE
    float Lighting = saturate(dot(Normal, normalize(vec3(1.0, 0.5, 1.0))));
    ColorOut.xyz = Albedo * Lighting;
    ColorOut.w = 1.0;
#elif DIFFUSE_ONLY
    ColorOut = vec4(Albedo, 0.0);
    NormalOut.xyz = Normal * 0.5 + 0.5;
    NormalOut.w = 1.0;

    // linearize and map to 0..255 range
    ColorOut.w = -0.003921569 / (gl_FragCoord.z - 1.003921569);
    ColorOut.w = log2(1.0 + saturate(length(EyeVec.xyz) / 200.0));
    ColorOut.w -= 1.0 / 255.0;
#else
    LightingOut = vec4(0.0);
    NormalOut = vec4(Normal * 0.5 + 0.5, 0.0);
    SpecularOut = vec4(Roughness, Metallic, 0.0, 0.0);
    AlbedoOut = vec4(Albedo, 1.0);
#endif
}

void main()
{
    vec3 Normal = texture(TexNormalmap, TexCoord).xyz * 2.0 - 1.0;
    Normal = normalize(Normal);

    vec2 scatter_uv;
    scatter_uv.x = saturate(length(EyeVec) / 1000.0);

    vec3 nEye = normalize(EyeVec);
    scatter_uv.y = 0.0; //nEye.x * 0.5 + 0.5;

    vec3 Color = vec3(0.1, 0.3, 0.1);
    vec3 grass = vec3(0.1, 0.3, 0.1);
    vec3 dirt = vec3(0.1, 0.1, 0.1);
    vec3 snow = vec3(0.8, 0.8, 0.8);

    float grass_snow = smoothstep(0.0, 0.15, (g_CamPos.y + EyeVec.y) / 200.0);
    vec3 base = mix(grass, snow, grass_snow);

    float edge = smoothstep(0.7, 0.75, Normal.y);
    Color = mix(dirt, base, edge);
    Color *= Color;

    float Roughness = 1.0 - edge * grass_snow;

    Resolve(Color, Normal, Roughness, 0.0);
}

