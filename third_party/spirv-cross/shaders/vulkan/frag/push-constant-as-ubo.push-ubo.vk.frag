#version 450

layout(push_constant, std140) uniform UBO
{
   float ubo[4];
};

layout(location = 0) out float FragColor;

void main()
{
   FragColor = ubo[1];
}
