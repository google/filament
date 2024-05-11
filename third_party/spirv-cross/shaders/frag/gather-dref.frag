#version 310 es
precision mediump float;

layout(binding = 0) uniform mediump sampler2DShadow uT;
layout(location = 0) in vec3 vUV;
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = textureGather(uT, vUV.xy, vUV.z);
}
