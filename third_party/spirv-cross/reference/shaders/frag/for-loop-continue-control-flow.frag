#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0);
    int i = 0;
    int _36;
    for (;;)
    {
        if (i < 3)
        {
            int a = i;
            FragColor[a] += float(i);
            if (false)
            {
                _36 = 1;
            }
            else
            {
                int _41 = i;
                i = _41 + 1;
                _36 = _41;
            }
            continue;
        }
        else
        {
            break;
        }
    }
}

