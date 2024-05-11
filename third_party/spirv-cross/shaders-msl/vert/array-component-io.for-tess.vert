#version 450

layout(location = 1, component = 0) out float A[2];
layout(location = 1, component = 2) out vec2 B[2];
layout(location = 0, component = 1) out float C[3];
layout(location = 0, component = 3) out float D;

layout(location = 1, component = 0) in float InA[2];
layout(location = 1, component = 2) in vec2 InB[2];
layout(location = 0, component = 1) in float InC[3];
layout(location = 0, component = 3) in float InD;
layout(location = 4) in vec4 Pos;

void main()
{
	gl_Position = Pos;
	A = InA;
	B = InB;
	C = InC;
	D = InD;
}
