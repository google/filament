#version 450
#extension GL_GOOGLE_cpp_style_line_directive : require
#line 100 "test.frag"

layout(location = 0) in vec3 iv;
layout(location = 0) out vec3 ov;

void func0()
{
	if (iv.x < 0.0)
		ov.x = 50.0;
	else
		ov.x = 60.0;
}

void func1()
{
	for (int i = 0; i < 4; i++)
	{
		func0();
		if (iv.y < 0.0)
			ov.y = 70.0;
		else
			ov.y = 80.0;
	}
}

void func2()
{
	for (int i = 0; i < 4; i++)
	{
		func0();
		func1();
		if (iv.z < 0.0)
			ov.z = 100.0;
		else
			ov.z = 120.0;
	}
}

void main ()
{
	func0();
	func1();
	func2();
}
