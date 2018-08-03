#version 450

layout(location = 0) flat in int counter;
layout(location = 0) out vec4 FragColor;

vec4 _21;

void main()
{
    vec4 _24;
    _24 = _21;
    vec4 _33;
    for (;;)
    {
        if (counter == 10)
        {
            _33 = vec4(10.0);
            break;
        }
        else
        {
            _33 = vec4(30.0);
            break;
        }
    }
    FragColor = _33;
}

