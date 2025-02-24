#version 310 es
uniform layout(binding = 1) mediump sampler2D sMask;
uniform layout(binding = 2) sampler2D sTexture;

layout(location = 0) in mediump vec2 vTexCoords;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float smp = texture(sTexture, vTexCoords).x;
	mediump float mask = texture(sMask, vTexCoords).x;
	const mediump vec3 hue = vec3(.25, 1, .25);
		
	oColor = vec4(hue * smp * mask, 1); 
}