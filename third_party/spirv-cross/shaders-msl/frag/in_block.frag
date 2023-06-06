#version 450

layout(location = 2) in VertexOut
{
    vec4 color;
    vec4 color2;
} inputs;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = inputs.color + inputs.color2;
}
