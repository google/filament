#version 320 es

layout(set = 1, binding = 0) uniform mediump sampler2D s2DMap;

layout(location = 0) in mediump vec2 TexCoords;
layout(location = 1) in mediump float LightIntensity;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	// Sample texture and shade fragment
	oColor = vec4(LightIntensity * texture(s2DMap, TexCoords).rgb, 1.0);
}
