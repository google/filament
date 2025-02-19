#version 450

layout(location = 0) flat in int vIndex;
layout(location = 0) out vec4 FragColor;

void main()
{
    int i;
    int j;
    int _33;
    int _34;
    if (vIndex != 0 && vIndex != 1 && vIndex != 11 && vIndex != 2 && vIndex != 3 && vIndex != 4 && vIndex != 5)
    {
        _33 = 2;
    }
    if (vIndex == 1 || vIndex == 11)
    {
        _34 = 1;
    }
    switch (vIndex)
    {
        case 0:
        {
            _33 = 3;
        }
        default:
        {
            j = _33;
            _34 = 0;
        }
        case 1:
        case 11:
        {
            j = _34;
        }
        case 2:
        {
            break;
        }
        case 3:
        {
            if (vIndex > 3)
            {
                i = 0;
                break;
            }
            else
            {
                break;
            }
        }
        case 4:
        {
        }
        case 5:
        {
            i = 0;
            break;
        }
    }
    FragColor = vec4(float(i));
}

