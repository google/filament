#version 450
layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D uTexture;
layout(binding = 1) uniform sampler1D uTexture2;

void main()
{
	FragColor = texelFetchOffset(uTexture, ivec2(gl_FragCoord.xy), 0, ivec2(1, 1));
	FragColor += texelFetchOffset(uTexture2, int(gl_FragCoord.x), 0, int(-1));
}
