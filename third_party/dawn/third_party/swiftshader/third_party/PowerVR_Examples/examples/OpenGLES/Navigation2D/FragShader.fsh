#version 300 es

uniform mediump vec4 myColor;

layout(location = 0) out mediump vec4 oColor;

void main(void)
{
	oColor = vec4(myColor.rgb, 1.0);
}