#version 450

layout(location = 0) in VertexData {
	flat float f;
	centroid vec4 g;
	flat int h;
	float i;
} vin;

layout(location = 4) in flat float f;
layout(location = 5) in centroid vec4 g;
layout(location = 6) in flat int h;
layout(location = 7) in sample float i;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vin.f + vin.g + float(vin.h) + vin.i + f + g + float(h) + i;
}
