#version 310 es
precision mediump float;

layout(std140, binding = 0) uniform UBO
{
	int some_value;
};

struct B
{
	float a;
	float b;
};

void partial_inout(inout vec4 x)
{
	x.x = 10.0;
}

void partial_inout(inout B b)
{
	b.b = 40.0;
}

// Make a complete write, but only conditionally ...
void branchy_inout(inout vec4 v)
{
	v.y = 20.0;
	if (some_value == 20)
	{
		v = vec4(50.0);
	}
}

void branchy_inout(inout B b)
{
	b.b = 20.0;
	if (some_value == 20)
	{
		b = B(10.0, 40.0);
	}
}

void branchy_inout_2(out vec4 v)
{
	if (some_value == 20)
	{
		v = vec4(50.0);
	}
	else
	{
		v = vec4(70.0);
	}
	v.y = 20.0;
}

void branchy_inout_2(out B b)
{
	if (some_value == 20)
	{
		b = B(10.0, 40.0);
	}
	else
	{
		b = B(70.0, 70.0);
	}
	b.b = 20.0;
}


void complete_inout(out vec4 x)
{
	x = vec4(50.0);
}

void complete_inout(out B b)
{
	b = B(100.0, 200.0);
}

void main()
{
	vec4 a = vec4(10.0);
	partial_inout(a);
	complete_inout(a);
	branchy_inout(a);
	branchy_inout_2(a);

	B b = B(10.0, 20.0);
	partial_inout(b);
	complete_inout(b);
	branchy_inout(b);
	branchy_inout_2(b);
}

