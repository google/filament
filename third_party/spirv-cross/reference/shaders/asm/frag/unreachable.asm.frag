#version 450

vec4 _21;

layout(location = 0) flat in int counter;
layout(location = 0) out vec4 FragColor;

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

