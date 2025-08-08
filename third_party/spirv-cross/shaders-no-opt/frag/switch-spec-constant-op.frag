#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 Attr;
layout(constant_id = 0) const int FOO = 4;

void main()
{
	switch (FOO + 4)
	{
		case 4:
			FragColor = vec4(10.0);
			break;

		case 6:
			FragColor = vec4(20.0);
			break;

		default:
			FragColor = vec4(0.0);
			break;
	}
}
