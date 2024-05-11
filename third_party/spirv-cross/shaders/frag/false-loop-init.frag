#version 310 es
precision mediump float;

layout(location = 0) in vec4 accum;
layout(location = 0) out vec4 result;

void main()
{
	result = vec4(0.0);
	uint j;
	for (int i = 0; i < 4; i += int(j))
	{
		if (accum.y > 10.0)
			j = 40u;
		else
			j = 30u;
		result += accum;
	}
}
