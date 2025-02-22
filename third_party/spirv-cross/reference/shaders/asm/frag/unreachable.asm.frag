#version 450

vec4 _44;

layout(location = 0) flat in int counter;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 _45;
    _45 = _44;
    vec4 _46;
    for (;;)
    {
        if (counter == 10)
        {
            _46 = vec4(10.0);
            break;
        }
        else
        {
            _46 = vec4(30.0);
            break;
        }
    }
    FragColor = _46;
}

