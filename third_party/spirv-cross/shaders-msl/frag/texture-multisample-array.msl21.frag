#version 450
layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2DMSArray uTexture;
layout(location = 0) flat in ivec3 vCoord;
layout(location = 1) flat in int vSample;

void main()
{
	FragColor = texelFetch(uTexture, vCoord, vSample);
}
