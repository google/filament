#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D sTexture;
layout(set = 0, binding = 1) uniform mediump sampler2D sNormalMap;

layout(location = 0) in mediump vec3 vLight;
layout(location = 1) in mediump vec2 vTexCoord;
layout(location = 2) in mediump float vOneOverAttenuation;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump vec3 fNormal = texture(sNormalMap, vTexCoord).rgb;
	mediump float fNDotL = clamp(dot((fNormal - 0.5) * 2.0, normalize(vLight)), 0.0, 1.0);
	fNDotL *= vOneOverAttenuation;
	
	oColor = texture(sTexture, vTexCoord) * fNDotL;
    oColor.a = 1.0;
}
