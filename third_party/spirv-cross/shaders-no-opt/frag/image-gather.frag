#version 450

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform sampler2D uSamp;
layout(set = 0, binding = 1) uniform sampler2DShadow uSampShadow;
layout(location = 0) in vec3 vUV;

void main()
{
	FragColor = textureGather(uSamp, vUV.xy, 0);
	FragColor += textureGather(uSamp, vUV.xy, 1);
	FragColor += textureGather(uSampShadow, vUV.xy, vUV.z);
}
