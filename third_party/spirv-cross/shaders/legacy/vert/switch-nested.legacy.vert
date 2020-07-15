#version 450

layout(set = 0, binding = 0) uniform UBO
{
	int func_arg;
	int inner_func_arg;
};

vec4 test_inner_func(bool b)
{
	if (b)
		return vec4(1.0);
	else
		return vec4(0.0);
}

vec4 test_func(bool b)
{
	if (b)
		return test_inner_func(inner_func_arg != 0);
	else
		return vec4(0.0);
}

void main()
{
	gl_Position = test_func(func_arg != 0);
}
