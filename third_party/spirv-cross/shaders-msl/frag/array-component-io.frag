#version 450

layout(location = 1, component = 0) out float A[2];
layout(location = 1, component = 2) out vec2 B[2];
layout(location = 0, component = 1) out float C[3];
layout(location = 0, component = 3) out float D;

layout(location = 1, component = 0) flat in float InA[2];
layout(location = 1, component = 2) flat in vec2 InB[2];
layout(location = 0, component = 1) flat in float InC[3];
layout(location = 3, component = 1) sample in float InD;
layout(location = 4, component = 2) noperspective in float InE;
layout(location = 5, component = 3) centroid in float InF;

void main()
{
	A = InA;
	B = InB;
	C = InC;
	D = InD + InE + InF;
}
