#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform sampler2D uTexture1;
layout(set = 0, binding = 1) uniform sampler2D uTexture2;
layout(set = 2, binding = 0) uniform sampler2D uTextureDiscrete1;
layout(set = 2, binding = 1) uniform sampler2D uTextureDiscrete2;

void main()
{
	FragColor = texture(uTexture2, vUV);
	FragColor += texture(uTextureDiscrete2, vUV);
}
