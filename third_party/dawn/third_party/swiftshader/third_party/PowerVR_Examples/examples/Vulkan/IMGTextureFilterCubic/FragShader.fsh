#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D tex;
layout(set = 0, binding = 1) uniform mediump sampler2D cubicTex;

layout(push_constant) uniform PushConsts
{
	highp mat4 MVPMatrix;
	mediump float WindowWidth;
}pushConstant;

layout(location = 0) in highp vec4 pos;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump vec2 texcoord = pos.xz * 0.01;

	mediump float imageCoordX = gl_FragCoord.x - 0.5;
	highp float xPosition = imageCoordX / pushConstant.WindowWidth;

	// Sample from the Image with Linear Sampling on the left hand side of the screen
	// Sample from the Image with Cubic Sampling on the right hand side of the screen
	oColor.rgb = xPosition < 0.5 ? texture(tex, texcoord).rgb : (xPosition > 0.497 && xPosition < 0.503) ? vec3(1.0, 1.0, 1.0) : texture(cubicTex, texcoord).rgb;
	oColor.a = 1.0;
}