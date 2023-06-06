#version 450

layout(binding = 0) uniform sampler2D uImage;

layout(location = 0) in vec4 v0;
layout(location = 0) out vec4 FragColor;

void main()
{
    float phi;
    vec4 _36;
    int _51;
    _51 = 0;
    phi = 1.0;
    _36 = vec4(1.0, 2.0, 1.0, 2.0);
    for (;;)
    {
        FragColor = _36;
        if (_51 < 4)
        {
            if (v0[_51] > 0.0)
            {
                vec2 _48 = vec2(phi);
                _51++;
                phi += 2.0;
                _36 = textureLod(uImage, _48, 0.0);
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

