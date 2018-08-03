#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2DMS uSampler;
layout(location = 0) out vec4 FragColor;

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	FragColor =
		texelFetch(uSampler, coord, 0) +
		texelFetch(uSampler, coord, 1) +
		texelFetch(uSampler, coord, 2) +
		texelFetch(uSampler, coord, 3);
}
