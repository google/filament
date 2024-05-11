#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0);
    for (int i = 0; i < 4; i++)
    {
        switch (i)
        {
            case 0:
            {
                FragColor.x += 1.0;
                break;
            }
            case 1:
            {
                FragColor.y += 3.0;
                break;
            }
            default:
            {
                FragColor.z += 3.0;
                break;
            }
        }
    }
}

