#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0);
    for (int _43 = 0; _43 < 3; )
    {
        FragColor[_43] += float(_43);
        _43++;
        continue;
    }
}

