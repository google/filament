#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 f4;
    mediump int c = int(f4.x);
    for (mediump int j = 0; j < c; j++)
    {
        switch (c)
        {
            case 0:
            {
                f4.y = 0.0;
                break;
            }
            case 1:
            {
                f4.y = 1.0;
                break;
            }
            default:
            {
                mediump int i = 0;
                for (;;)
                {
                    mediump int _48 = i;
                    mediump int _50 = _48 + 1;
                    i = _50;
                    if (_48 < c)
                    {
                        f4.y += 0.5;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                continue;
            }
        }
        f4.y += 0.5;
    }
    fragColor = f4;
}

