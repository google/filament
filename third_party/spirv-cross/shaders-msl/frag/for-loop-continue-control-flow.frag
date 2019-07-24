#version 450
layout(location = 0) out vec4 FragColor;
void main()
{
	FragColor = vec4(0.0);
	for (int i = 0; i < 3; (0 > 1) ? 1 : i ++)
	{
		int a = i;
		FragColor[a] += float(i);
	}
}
