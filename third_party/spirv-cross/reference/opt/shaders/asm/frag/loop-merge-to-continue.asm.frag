#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 v0;

void main()
{
    FragColor = vec4(1.0);
    int _50;
    _50 = 0;
    for (; _50 < 4; _50++)
    {
        for (int _51 = 0; _51 < 4; )
        {
            FragColor += vec4(v0[(_50 + _51) & 3]);
            _51++;
            continue;
        }
    }
}

