#version 310 es
precision mediump float;
layout(location = 0) out vec4 FragColor;

void main()
{
	float v;
	bool written = false;
	for (int j = 0; j < 10; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			float w = 0.0;
			if (written)
				w += v;
			else
				v = 20.0;

			v += float(i);
			written = true;
		}
	}
	FragColor = vec4(1.0);
}
