#version 450
layout(location = 0) in mat3 vMatrix;
layout(location = 4) in Vert
{
	mat3 wMatrix;
	vec4 wTmp;
	float arr[4];
};

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = wMatrix[0].xxyy + wTmp + vMatrix[1].yyzz;
	for (int i = 0; i < 4; i++)
		FragColor += arr[i];
}
