#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D sTexture;
layout(set = 0, binding = 1) uniform mediump sampler2D sMask;

layout(location = 0) in mediump vec2 vTexCoords;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float smp = texture(sTexture, vTexCoords).x;
	mediump float mask = texture(sMask, vTexCoords).x;
	const mediump vec3 hue = vec3(.25, 1, .25);
	
	oColor = vec4(hue * smp * mask, 1); 
}