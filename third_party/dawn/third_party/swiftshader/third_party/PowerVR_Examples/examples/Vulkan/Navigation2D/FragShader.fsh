#version 320 es

layout(location = 0) out mediump vec4 oColor;

layout(std140, set = 1, binding = 0) uniform DynamicData
{
	uniform mediump vec4 myColor;
};

void main(void)
{
	oColor = vec4(myColor.rgb, 1.0);
}
