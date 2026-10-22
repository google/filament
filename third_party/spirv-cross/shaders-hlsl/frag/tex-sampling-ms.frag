#version 450

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2DMS uTex;

void main()
{
	FragColor =
		texelFetch(uTex, ivec2(gl_FragCoord.xy), 0);
	FragColor +=
		texelFetch(uTex, ivec2(gl_FragCoord.xy), 1);
	FragColor +=
		texelFetch(uTex, ivec2(gl_FragCoord.xy), 2);
	FragColor +=
		texelFetch(uTex, ivec2(gl_FragCoord.xy), 3);
}
