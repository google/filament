#version 450

const double posinf = 1.0lf / 0.0lf;
const double neginf = -1.0lf / 0.0lf;
const double nan = 0.0lf / 0.0lf;

layout(location = 0) out vec3 FragColor;
layout(location = 0) flat in double vTmp;

void main()
{
	FragColor = vec3(dvec3(posinf, neginf, nan) + vTmp);
}
