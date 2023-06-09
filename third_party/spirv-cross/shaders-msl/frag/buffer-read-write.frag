#version 450

layout(rgba8, binding = 0) uniform readonly imageBuffer buf;
layout(rgba8, binding = 1) uniform writeonly imageBuffer bufOut;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = imageLoad(buf, 0);
	imageStore(bufOut, int(gl_FragCoord.x), FragColor);
}
