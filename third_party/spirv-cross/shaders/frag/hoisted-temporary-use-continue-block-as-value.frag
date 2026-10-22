#version 310 es
precision mediump float;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in int vA;
layout(location = 1) flat in int vB;

void main()
{
	FragColor = vec4(0.0);

	int k = 0;
	int j;
	for (int i = 0; i < vA; i += j)
	{
		if ((vA + i) == 20)
			k = 50;
		else if ((vB + i) == 40)
			k = 60;

		j = k + 10;
		FragColor += 1.0;
	}
}
