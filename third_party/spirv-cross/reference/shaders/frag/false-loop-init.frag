#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 result;
layout(location = 0) in vec4 accum;

void main()
{
    result = vec4(0.0);
    mediump uint j;
    for (mediump int i = 0; i < 4; i += int(j))
    {
        if (accum.y > 10.0)
        {
            j = 40u;
        }
        else
        {
            j = 30u;
        }
        result += accum;
    }
}

