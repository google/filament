#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vInput;

layout(set = 0, binding = 0) uniform UBO
{
	int cond;
	int cond2;
};

void main()
{
	FragColor = vec4(10.0);
	switch (cond)
	{
	case 1:
		if (cond2 < 50)
			break;
		else
			discard;

		break;

	default:
		FragColor = vec4(20.0);
		break;
	}
}

