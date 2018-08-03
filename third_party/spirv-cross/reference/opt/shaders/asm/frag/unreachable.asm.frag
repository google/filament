#version 450

layout(location = 0) flat in int counter;
layout(location = 0) out vec4 FragColor;

void main()
{
    bool _29;
    for (;;)
    {
        _29 = counter == 10;
        if (_29)
        {
            break;
        }
        else
        {
            break;
        }
    }
    FragColor = mix(vec4(30.0), vec4(10.0), bvec4(_29));
}

