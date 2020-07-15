#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 0) uniform sampler2D uSampler;

void main()
{
	FragColor = textureProj(uSampler, vec3(vUV, 5.0));
	FragColor += texture(uSampler, vUV, 3.0);
	FragColor += textureLod(uSampler, vUV, 2.0);
	FragColor += textureGrad(uSampler, vUV, vec2(4.0), vec2(5.0));
}
