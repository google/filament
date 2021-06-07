#version 450

layout(vertices = 4) out;
patch out P
{
	layout(location = 0) float a;
	layout(location = 2) float b;
};

out C
{
	layout(location = 1) float a;
	layout(location = 3) float b;
} c[];

void write_in_function()
{
	a = 1.0;
	b = 2.0;
	c[gl_InvocationID].a = 3.0;
	c[gl_InvocationID].b = 4.0;
	gl_out[gl_InvocationID].gl_Position = vec4(1.0);
}

void main()
{
	write_in_function();
}
