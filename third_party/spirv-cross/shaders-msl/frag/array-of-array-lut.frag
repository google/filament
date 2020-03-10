#version 450

layout(location = 0) out float vOutput;
layout(location = 0) flat in int vIndex1;
layout(location = 1) flat in int vIndex2;

const float FOO[2][3] = float[][](float[](1.0, 2.0, 3.0), float[](4.0, 5.0, 6.0));

void main()
{
	vOutput = FOO[vIndex1][vIndex2];
}
