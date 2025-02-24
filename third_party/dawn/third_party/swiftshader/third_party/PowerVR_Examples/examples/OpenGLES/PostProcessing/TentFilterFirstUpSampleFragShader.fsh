#version 310 es

layout(binding = 0) uniform mediump sampler2D sDownsampledImage;

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 0) out mediump float oColor;

void main()
{
	oColor = texture(sDownsampledImage, vTexCoord).r;
}