#version 450 core
#extension GL_EXT_texture_offset_non_const : enable
layout(location = 0) in highp vec4 a_position;
layout(location = 4) in highp vec2 a_in0;
layout(location = 5) in highp float a_in1;
layout(location = 10) in highp ivec2 offsetValue;
layout(location = 0) out mediump vec4 v_color0;
layout(location = 1) out mediump vec4 v_color1;
layout(location = 2) out mediump vec4 v_color2;
layout(location = 3) out mediump vec4 v_color3;
layout(location = 4) out mediump vec4 v_color4;
layout(location = 5) out mediump vec4 v_color5;
layout(location = 6) out mediump vec4 v_color6;
layout(set = 0, binding = 0) uniform highp sampler2D u_sampler;
layout(set = 0, binding = 1) uniform buf0 { highp vec4 u_scale; };
layout(set = 0, binding = 2) uniform buf1 { highp vec4 u_bias; };
out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
	gl_Position = a_position;
	v_color0 = vec4(textureOffset(u_sampler, a_in0, offsetValue))*u_scale + u_bias;
	v_color1 = vec4(texelFetchOffset(u_sampler, ivec2(a_in0), int(a_in1), offsetValue))*u_scale + u_bias;
	v_color2 = vec4(textureProjOffset(u_sampler, vec3(a_in0, 1.0), offsetValue))*u_scale + u_bias;
	v_color3 = vec4(textureLodOffset(u_sampler, a_in0, a_in1, offsetValue))*u_scale + u_bias;
	v_color4 = vec4(textureProjLodOffset(u_sampler, vec3(a_in0, 1.0), a_in1, offsetValue))*u_scale + u_bias;
	v_color5 = vec4(textureGradOffset(u_sampler, a_in0, vec2(a_in1, a_in1), vec2(a_in1, a_in1), offsetValue))*u_scale + u_bias;
	v_color6 = vec4(textureProjGradOffset(u_sampler, vec3(a_in0, 1.0), vec2(a_in1, a_in1), vec2(a_in1, a_in1), offsetValue))*u_scale + u_bias;
}
