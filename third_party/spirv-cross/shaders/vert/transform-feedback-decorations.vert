#version 450
layout(xfb_stride = 32, xfb_offset = 16, xfb_buffer = 2, location = 0) out vec4 vFoo;

layout(xfb_buffer = 1, xfb_stride = 20) out gl_PerVertex
{
	layout(xfb_offset = 4) vec4 gl_Position;
	float gl_PointSize;
};

layout(xfb_buffer = 3) out VertOut
{
	layout(xfb_stride = 16, xfb_offset = 0, location = 1) vec4 vBar;
};

void main()
{
	gl_Position = vec4(1.0);
	vFoo = vec4(3.0);
	vBar = vec4(5.0);
}
