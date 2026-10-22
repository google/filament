#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int vA;
layout(location = 1) flat in mediump int vB;

void main()
{
    FragColor = vec4(0.0);
    mediump int k = 0;
    mediump int j;
    for (mediump int i = 0; i < vA; i += j)
    {
        if ((vA + i) == 20)
        {
            k = 50;
        }
        else
        {
            if ((vB + i) == 40)
            {
                k = 60;
            }
        }
        j = k + 10;
        FragColor += vec4(1.0);
    }
}

