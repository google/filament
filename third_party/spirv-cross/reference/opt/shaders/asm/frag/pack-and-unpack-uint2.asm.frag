#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    uvec2 unpacked = uvec2(18u, 52u);
    FragColor = vec4(float(unpacked.x), float(unpacked.y), 1.0, 1.0);
}

