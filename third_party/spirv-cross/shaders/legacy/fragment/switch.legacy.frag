#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in float vIndexF;

void main()
{
	int vIndex = int(vIndexF);
	vec4 v = vec4(0.0);
	switch (vIndex)
	{
	case 2:
		v = vec4(0, 2, 3, 4);
		break;
	case 4:
	case 5:
		v = vec4(1, 2, 3, 4);
		break;
	case 8:
	case 9:
		v = vec4(40, 20, 30, 40);
		break;
	case 10:
		v = vec4(10.0);
	case 11:
		v += 1.0;
	case 12:
		v += 2.0;
		break;
	default:
		v = vec4(10, 20, 30, 40);
		break;
	}

	vec4 w = vec4(20.0);
	switch (vIndex)
	{
	case 10:
	case 20:
		w = vec4(40.0);
	}
	FragColor = v + w;
}
