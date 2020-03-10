#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out highp vec4 _GLF_color;

void main()
{
    for (;;)
    {
        bool _32;
        for (;;)
        {
            if (gl_FragCoord.x != gl_FragCoord.x)
            {
                _32 = true;
                break;
            }
            if (false)
            {
                continue;
            }
            else
            {
                _32 = false;
                break;
            }
        }
        if (_32)
        {
            break;
        }
        _GLF_color = vec4(1.0, 0.0, 0.0, 1.0);
        break;
    }
}

