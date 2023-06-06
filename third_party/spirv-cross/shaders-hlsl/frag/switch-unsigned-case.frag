#version 310 es
precision mediump float;

#define ENUM_0 0u
#define ENUM_1 1u

layout(set = 0, binding = 0) uniform Buff
{
	uint TestVal;
};

layout(location = 0) out vec4 fsout_Color;

void main()
{
	fsout_Color = vec4(1.0);
	switch (TestVal)
	{
		case ENUM_0:
			fsout_Color = vec4(0.1);
			break;
		case ENUM_1:
			fsout_Color = vec4(0.2);
			break;
	}
}
