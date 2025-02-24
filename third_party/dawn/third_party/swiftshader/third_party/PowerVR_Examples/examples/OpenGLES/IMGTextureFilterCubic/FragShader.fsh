#version 310 es

layout(location=0) uniform mediump sampler2D tex;
layout(location=1) uniform mediump sampler2D cubicTex;

uniform mediump float WindowWidth;

layout(location=0) out mediump vec4 outColor;

in highp vec4 pos;

void main()
{
	mediump vec2 texcoord = pos.xz * 0.01;

	mediump float imageCoordX = gl_FragCoord.x - 0.5;
	highp float xPosition = imageCoordX / WindowWidth;

	mediump vec3 color = xPosition < 0.5 ? texture(tex, texcoord).rgb : (xPosition > 0.497 && xPosition < 0.503) ? vec3(1.0, 1.0, 1.0) : texture(cubicTex, texcoord).rgb;
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Gamma correction
#endif	
	outColor.rgb = color;
	outColor.a = 1.0;
}