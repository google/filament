#version 450
layout(location = 0, component = 0) in vec2 v0;
layout(location = 0, component = 2) in float v1;
layout(location = 0) out vec2 FragColor;

in Vertex
{
	layout(location = 1, component = 2) in float v3;
};

void main()
{
	FragColor = v0 + v1 + v3;
}
