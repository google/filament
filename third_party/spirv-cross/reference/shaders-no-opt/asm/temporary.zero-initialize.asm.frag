#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int vA;
layout(location = 1) flat in mediump int vB;

void main()
{
    FragColor = vec4(0.0);
    mediump int _49 = 0;
    mediump int _58 = 0;
    for (mediump int _57 = 0, _60 = 0; _57 < vA; _60 = _58, _57 += _49)
    {
        if ((vA + _57) == 20)
        {
            _58 = 50;
        }
        else
        {
            _58 = ((vB + _57) == 40) ? 60 : _60;
        }
        _49 = _58 + 10;
        FragColor += vec4(1.0);
    }
}

