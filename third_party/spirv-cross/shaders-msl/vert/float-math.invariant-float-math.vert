#version 450

layout(set = 0, binding = 0) uniform Matrices
{
	mat4   vpMatrix;
	mat4   wMatrix;
	mat4x3 wMatrix4x3;
	mat3x4 wMatrix3x4;
};

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InNormal;

layout(location = 0) out vec3 OutNormal;
layout(location = 1) out vec4 OutWorldPos[4];

void main()
{
	gl_Position = vpMatrix * wMatrix * vec4(InPos, 1);
	OutWorldPos[0] = wMatrix * vec4(InPos, 1);
	OutWorldPos[1] = vec4(InPos, 1) * wMatrix;
	OutWorldPos[2] = wMatrix3x4 * InPos;
	OutWorldPos[3] = InPos * wMatrix4x3;
	OutNormal = (wMatrix * vec4(InNormal, 0)).xyz;
}
