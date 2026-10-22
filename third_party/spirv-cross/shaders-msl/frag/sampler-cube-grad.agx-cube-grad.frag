#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in vec3 vTex;
layout(binding = 0) uniform samplerCube uSampler;

void main()
{
	FragColor += textureGrad(uSampler, vTex, vec3(5.0), vec3(8.0));
}
