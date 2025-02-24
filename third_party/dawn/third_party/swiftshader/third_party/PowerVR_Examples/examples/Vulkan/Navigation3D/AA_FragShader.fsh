#version 320 es

layout (set = 2, binding = 0) uniform mediump sampler2D sTexture;

layout(location = 0) out mediump vec4 oColor;

layout(location = 0) in mediump vec4 fragColor;
layout(location = 1) in mediump vec2 texCoordOut;

void main(void)
{
	mediump vec4 texColor = texture(sTexture, texCoordOut);
	oColor = vec4(fragColor.rgb * texColor.r,texColor.a);
}