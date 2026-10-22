#version 310 es
precision mediump float;

struct Foo
{
	vec4 a;
	vec4 b;
};

layout(binding = 0) buffer UBO
{
	vec4 results[1024];
};

layout(binding = 1) uniform highp isampler2D Buf;
layout(location = 0) flat in int vIn;
layout(location = 1) flat in int vIn2;

layout(location = 0) out vec4 FragColor;

void main()
{
	ivec4 coords = texelFetch(Buf, ivec2(gl_FragCoord.xy), 0);
	vec4 foo = results[coords.x % 16];

	int c = vIn * vIn;
	int d = vIn2 * vIn2;
	FragColor = foo + foo + results[c + d];
}
