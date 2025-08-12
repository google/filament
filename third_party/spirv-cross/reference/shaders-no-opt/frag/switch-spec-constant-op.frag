#version 450

#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 4
#endif
const int FOO = SPIRV_CROSS_CONSTANT_ID_0;
const int _9 = (FOO + 4);

layout(location = 0) out vec4 FragColor;

void main()
{
    switch (_9)
    {
        case 4:
        {
            FragColor = vec4(10.0);
            break;
        }
        case 6:
        {
            FragColor = vec4(20.0);
            break;
        }
        default:
        {
            FragColor = vec4(0.0);
            break;
        }
    }
}

