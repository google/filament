#version 310 es
precision mediump float;
layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D uTexture;

void main()
{
	FragColor = texelFetchOffset(uTexture, ivec2(gl_FragCoord.xy), 0, ivec2(1, 1));
	FragColor += texelFetchOffset(uTexture, ivec2(gl_FragCoord.xy), 0, ivec2(-1, 1));
}
