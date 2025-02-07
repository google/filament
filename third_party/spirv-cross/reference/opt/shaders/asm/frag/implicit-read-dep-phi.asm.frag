#version 450

layout(binding = 0) uniform sampler2D uImage;

layout(location = 0) in vec4 v0;
layout(location = 0) out vec4 FragColor;

void main()
{
    float phi;
    vec4 _45;
    int _57;
    _57 = 0;
    phi = 1.0;
    _45 = vec4(1.0, 2.0, 1.0, 2.0);
    for (;;)
    {
        FragColor = _45;
        if (_57 < 4)
        {
            if (v0[_57] > 0.0)
            {
                vec2 _43 = vec2(phi);
                _57++;
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

