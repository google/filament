#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D sDownsampledImage;

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 0) out mediump float oColor;

void main()
{
	oColor = texture(sDownsampledImage, vTexCoord).r;
}