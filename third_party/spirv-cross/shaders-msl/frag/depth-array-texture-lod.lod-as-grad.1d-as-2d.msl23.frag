#version 450 core
layout(location = 0) out mediump vec4 o_color;
layout(location = 0) in highp vec3 v_texCoord;
layout(location = 1) in highp float v_lodBias;
layout(set = 0, binding = 0) uniform highp sampler1DArrayShadow u_sampler;
layout(set = 0, binding = 1) uniform buf0 { highp vec4 u_scale; };
layout(set = 0, binding = 2) uniform buf1 { highp vec4 u_bias; };

void main()
{
	o_color = vec4(textureLod(u_sampler, v_texCoord, v_lodBias), 0.0, 0.0, 1.0);
}
