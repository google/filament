#version 450

layout(location = 0) out float FragColor;

void main()
{
    FragColor = float((10u > 20u) ? 30u : 50u);
}

