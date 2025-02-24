#version 310 es

uniform mediump sampler2D sTexture;

in mediump vec2 vTexCoord;
in mediump vec3 vWorldNormal;
in mediump vec3 vLightDir;
in mediump float vOneOverAttenuation;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float lightIntensity = clamp(dot(normalize(vLightDir), normalize(vWorldNormal)), 0.0, 1.0);
    lightIntensity *= vOneOverAttenuation;

	mediump vec3 color = texture(sTexture, vTexCoord).rgb * lightIntensity;
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif	
	oColor = vec4(color, 1.0);
}
