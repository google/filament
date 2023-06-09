#version 450

layout(location = 0) out V
{
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d;
};

void main()
{
	gl_Position = vec4(1.0);
	a = vec4(2.0);
	b = vec4(3.0);
}
