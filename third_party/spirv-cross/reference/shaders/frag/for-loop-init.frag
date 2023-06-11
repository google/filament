#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out mediump int FragColor;

void main()
{
    FragColor = 16;
    for (mediump int i = 0; i < 25; i++)
    {
        FragColor += 10;
    }
    for (mediump int i_1 = 1, j = 4; i_1 < 30; i_1++, j += 4)
    {
        FragColor += 11;
    }
    mediump int k = 0;
    for (; k < 20; k++)
    {
        FragColor += 12;
    }
    k += 3;
    FragColor += k;
    mediump int l;
    if (k == 40)
    {
        l = 0;
        for (; l < 40; l++)
        {
            FragColor += 13;
        }
        return;
    }
    else
    {
        l = k;
        FragColor += l;
    }
    mediump ivec2 i_2 = ivec2(0);
    for (; i_2.x < 10; i_2.x += 4)
    {
        FragColor += i_2.y;
    }
    mediump int o = k;
    for (mediump int m = k; m < 40; m++)
    {
        FragColor += m;
    }
    FragColor += o;
}

