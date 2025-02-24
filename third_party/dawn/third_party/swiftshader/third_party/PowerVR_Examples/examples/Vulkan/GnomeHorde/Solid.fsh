#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D tex;

layout(location = 0) in mediump vec2 uvs;
layout(location = 1) in mediump float nDotL;

layout(location = 0) out mediump vec4 outColor;

void main()
{
	mediump vec3 tex_res = texture(tex, uvs).rgb;
	mediump vec3 ambient = vec3(0.1, 0.1, 0.1);
	outColor = vec4((ambient + nDotL) * tex_res, 1.0);
}