#version 450
layout(location = 0) out mat3 vMatrix;
layout(location = 0) in mat3 Matrix;
layout(location = 4) in vec4 Pos;

layout(location = 4) out Vert
{
	float arr[3];
	mat3 wMatrix;
	vec4 wTmp;
};

void main()
{
	vMatrix = Matrix;
	wMatrix = Matrix;
	arr[0] = 1.0;
	arr[1] = 2.0;
	arr[2] = 3.0;
	wTmp = Pos;
	gl_Position = Pos;
}
