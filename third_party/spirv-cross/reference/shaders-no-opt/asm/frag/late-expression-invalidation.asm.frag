#version 310 es
precision mediump float;
precision highp int;

const mat4 _34[4] = mat4[](mat4(vec4(1.0), vec4(1.0), vec4(1.0), vec4(1.0)), mat4(vec4(1.0), vec4(1.0), vec4(1.0), vec4(1.0)), mat4(vec4(1.0), vec4(1.0), vec4(1.0), vec4(1.0)), mat4(vec4(1.0), vec4(1.0), vec4(1.0), vec4(1.0)));

layout(location = 0) out highp vec4 _GLF_color;

void main()
{
    for (;;)
    {
        if (gl_FragCoord.x < 10.0)
        {
            _GLF_color = vec4(1.0, 0.0, 0.0, 1.0);
            break;
        }
        for (int _46 = 0; _46 < 4; _46++)
        {
            int _53;
            _53 = 0;
            bool _56;
            for (;;)
            {
                _56 = _53 < 4;
                if (_56)
                {
                    if (distance(vec2(1.0), vec2(1.0) / vec2(_34[int(_56)][_46].w)) < 1.0)
                    {
                        _GLF_color = vec4(1.0);
                        int _54 = _53 + 1;
                        _53 = _54;
                        continue;
                    }
                    else
                    {
                        int _54 = _53 + 1;
                        _53 = _54;
                        continue;
                    }
                    int _54 = _53 + 1;
                    _53 = _54;
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
        break;
    }
}

