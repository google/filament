#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 result;
layout(location = 0) in vec4 accum;

void main()
{
    result = vec4(0.0);
    for (mediump int _48 = 0; _48 < 4; )
    {
        result += accum;
        _48 += int((accum.y > 10.0) ? 40u : 30u);
        continue;
    }
}

