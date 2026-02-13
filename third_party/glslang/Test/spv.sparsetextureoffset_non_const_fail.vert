#version 450 core
#extension GL_ARB_sparse_texture2 : enable
layout(location = 0) in highp vec4 a_position;
layout(location = 4) in highp vec2 a_in0;
layout(location = 5) in highp float a_in1;
layout(location = 10) in highp ivec2 offsetValue;
layout(location = 0) out mediump vec4 v_color0;
layout(location = 1) out mediump vec4 v_color1;
layout(location = 2) out mediump vec4 v_color2;
layout(location = 3) out mediump vec4 v_color3;
layout(set = 0, binding = 0) uniform highp sampler2D u_sampler;
layout(set = 0, binding = 1) uniform buf0 { highp vec4 u_scale; };
layout(set = 0, binding = 2) uniform buf1 { highp vec4 u_bias; };
out gl_PerVertex {
	vec4 gl_Position;
};

void main()
{
	gl_Position = a_position;
	vec4 aux0;
	vec4 aux1;
	vec4 aux2;
	vec4 aux3;

	int ret0 = sparseTextureOffsetARB(u_sampler, a_in0, offsetValue, aux0);
	int ret1 = sparseTexelFetchOffsetARB(u_sampler, ivec2(a_in0), int(a_in1), offsetValue, aux1);
	int ret2 = sparseTextureLodOffsetARB(u_sampler, a_in0, a_in1, offsetValue, aux2);
	int ret3 = sparseTextureGradOffsetARB(u_sampler, a_in0, vec2(a_in1, a_in1), vec2(a_in1, a_in1), offsetValue, aux3);

	v_color0 = aux0 * u_scale + u_bias;
	v_color1 = aux1 * u_scale + u_bias;
	v_color2 = aux2 * u_scale + u_bias;
	v_color3 = aux3 * u_scale + u_bias;
}
