#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int vA;
layout(location = 1) flat in mediump int vB;

void main()
{
    FragColor = vec4(0.0);
    mediump int _10 = 0;
    mediump int _15 = 0;
    for (mediump int _16 = 0, _17 = 0; _16 < vA; _17 = _15, _16 += _10)
    {
        if ((vA + _16) == 20)
        {
            _15 = 50;
        }
        else
        {
            _15 = ((vB + _16) == 40) ? 60 : _17;
        }
        _10 = _15 + 10;
        FragColor += vec4(1.0);
    }
}

