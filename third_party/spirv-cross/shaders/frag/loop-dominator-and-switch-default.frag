#version 310 es
precision mediump float;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 f4;
    int c = int(f4.x);

    for (int j = 0; j < c; j++)
    {
        switch (c)
        {
            case 0:
                f4.y = 0.0;
                break;
            case 1:
                f4.y = 1.0;
                break;
            default:
            {
                int i = 0;
                while (i++ < c) {
                    f4.y += 0.5;
                }
                continue;
            }
        }
        f4.y += 0.5;
    }

    fragColor = f4;
}
