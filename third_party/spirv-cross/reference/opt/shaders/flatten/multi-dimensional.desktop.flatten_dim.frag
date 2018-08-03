#version 450

layout(binding = 0) uniform sampler2D uTextures[2 * 3 * 1];

layout(location = 1) in vec2 vUV;
layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in int vIndex;

void main()
{
    int _92;
    _92 = 0;
    vec4 values3[2 * 3 * 1];
    for (; _92 < 2; _92++)
    {
        int _93;
        _93 = 0;
        for (; _93 < 3; _93++)
        {
            for (int _95 = 0; _95 < 1; )
            {
                values3[_92 * 3 * 1 + _93 * 1 + _95] = texture(uTextures[_92 * 3 * 1 + _93 * 1 + _95], vUV);
                _95++;
                continue;
            }
        }
    }
    FragColor = (values3[1 * 3 * 1 + 2 * 1 + 0] + values3[0 * 3 * 1 + 2 * 1 + 0]) + values3[(vIndex + 1) * 3 * 1 + 2 * 1 + vIndex];
}

