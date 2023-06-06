#version 310 es

layout(binding = 0) uniform Buffer
{
	layout(row_major) highp mat4 HP;
	layout(row_major) mediump mat4 MP;
};

layout(binding = 1) uniform Buffer2
{
	layout(row_major) mediump mat4 MP2;
};


layout(location = 0) in vec4 Hin;
layout(location = 1) in mediump vec4 Min;
layout(location = 0) out vec4 H;
layout(location = 1) out mediump vec4 M;
layout(location = 2) out mediump vec4 M2;

void main()
{
	gl_Position = vec4(1.0);
	H = HP * Hin;
	M = MP * Min;
	M2 = MP2 * Min;
}

