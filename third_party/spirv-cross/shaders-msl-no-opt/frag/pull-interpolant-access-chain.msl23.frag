#version 450
layout(location = 0) centroid in vec4 a[2];
layout(location = 2) centroid in vec4 b[2];
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor.x = interpolateAtOffset(a[0].x, vec2(0.5));
	FragColor.y = interpolateAtOffset(a[1].y, vec2(0.5));
	FragColor.z = interpolateAtOffset(b[0].z, vec2(0.5));
	FragColor.w = interpolateAtOffset(b[1].w, vec2(0.5));
}
