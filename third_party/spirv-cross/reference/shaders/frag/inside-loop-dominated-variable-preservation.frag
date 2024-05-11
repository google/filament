#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;

void main()
{
    bool written = false;
    float v;
    for (mediump int j = 0; j < 10; j++)
    {
        for (mediump int i = 0; i < 4; i++)
        {
            float w = 0.0;
            if (written)
            {
                w += v;
            }
            else
            {
                v = 20.0;
            }
            v += float(i);
            written = true;
        }
    }
    FragColor = vec4(1.0);
}

