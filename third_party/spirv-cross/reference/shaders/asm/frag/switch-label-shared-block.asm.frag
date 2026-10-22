#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) flat in mediump int vIndex;
layout(location = 0) out float FragColor;

void main()
{
    highp float _27;
    switch (vIndex)
    {
        case 0:
        case 2:
        {
            _27 = 1.0;
            break;
        }
        default:
        {
            _27 = 3.0;
            break;
        }
        case 8:
        {
            _27 = 8.0;
            break;
        }
    }
    FragColor = _27;
}

