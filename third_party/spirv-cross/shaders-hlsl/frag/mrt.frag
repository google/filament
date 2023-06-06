#version 310 es
precision mediump float;

layout(location = 0) out vec4 RT0;
layout(location = 1) out vec4 RT1;
layout(location = 2) out vec4 RT2;
layout(location = 3) out vec4 RT3;

void main()
{
	RT0 = vec4(1.0);
	RT1 = vec4(2.0);
	RT2 = vec4(3.0);
	RT3 = vec4(4.0);
}
