#version 310 es

uniform mediump sampler2D sTexture;
uniform mediump sampler2D sNormalMap;

in highp vec3 worldPosition;
in mediump vec3 vLight;
in mediump vec2 vTexCoord;
in mediump float vOneOverAttenuation;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump vec3 fNormal = texture(sNormalMap, vTexCoord).rgb;
	mediump float fNDotL = clamp(dot((fNormal - 0.5) * 2.0, normalize(vLight)), 0.0, 1.0);
	fNDotL *= vOneOverAttenuation;

	mediump vec3 color = texture(sTexture, vTexCoord).rgb * fNDotL;

#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif
	oColor = vec4(color, 1.0);
}
