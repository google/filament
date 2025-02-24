#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D tex;

layout(location = 0) in mediump vec2 uvs;

layout(location = 0) out mediump vec4 outColor;

void main()
{
	outColor = vec4(0.03, 0.0, 0.05, texture(tex, uvs).r);
}