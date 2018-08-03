#version 310 es
precision mediump float;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vColor;

void main()
{
	FragColor = gl_FragCoord + vColor;
	gl_FragDepth = 0.5;
}
