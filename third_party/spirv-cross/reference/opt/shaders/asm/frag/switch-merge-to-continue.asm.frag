#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0);
    for (int _52 = 0; _52 < 4; _52++)
    {
        switch (_52)
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
        continue;
    }
}

