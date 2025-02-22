#version 450

layout(binding = 0) uniform sampler2D uImage;

layout(location = 0) in vec4 v0;
layout(location = 0) out vec4 FragColor;

void main()
{
    int i = 0;
    float phi;
    vec4 _45;
    phi = 1.0;
    _45 = vec4(1.0, 2.0, 1.0, 2.0);
    for (;;)
    {
        FragColor = _45;
        if (i < 4)
        {
            if (v0[i] > 0.0)
            {
                vec2 _43 = vec2(phi);
                i++;
                phi += 2.0;
                _45 = textureLod(uImage, _43, 0.0);
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

