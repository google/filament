#version 310 es

layout(location = 0) in vec4 a;
layout(location = 1) in vec4 b;
layout(location = 2) in vec4 c;
layout(location = 3) in vec4 d;

struct VOut
{
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d;
};

layout(location = 0) out VOut vout;

void emit_result(VOut v)
{
	vout = v;
}

void main()
{
	emit_result(VOut(a, b, c, d));
}
