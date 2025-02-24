#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D sTexture;
layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in mediump vec3 vWorldNormal;
layout(location = 2) in mediump vec3 vLightDir;
layout(location = 3) in mediump float vOneOverAttenuation;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float lightIntensity = clamp(dot(normalize(vLightDir), normalize(vWorldNormal)), 0.0, 1.0);
    lightIntensity *= vOneOverAttenuation;

    oColor = texture(sTexture, vTexCoord);
	oColor.xyz = oColor.xyz * lightIntensity;
    oColor.a = 1.0;
}
