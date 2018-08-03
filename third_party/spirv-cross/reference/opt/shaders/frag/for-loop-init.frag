#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out mediump int FragColor;

ivec2 _184;
int _199;

void main()
{
    FragColor = 16;
    for (int _168 = 0; _168 < 25; )
    {
        FragColor += 10;
        _168++;
        continue;
    }
    for (int _169 = 1; _169 < 30; )
    {
        FragColor += 11;
        _169++;
        continue;
    }
    int _170;
    _170 = 0;
    for (; _170 < 20; )
    {
        FragColor += 12;
        _170++;
        continue;
    }
    mediump int _62 = _170 + 3;
    FragColor += _62;
    bool _68 = _62 == 40;
    if (_68)
    {
        for (int _171 = 0; _171 < 40; )
        {
            FragColor += 13;
            _171++;
            continue;
        }
    }
    else
    {
        FragColor += _62;
    }
    bool _213 = _68 ? true : false;
    if (!_213)
    {
        ivec2 _177;
        _177 = mix(ivec2(0), mix(_184, _184, bvec2(_68)), bvec2(_213));
        for (; _177.x < 10; )
        {
            FragColor += _177.y;
            ivec2 _167 = _177;
            _167.x = _177.x + 4;
            _177 = _167;
            continue;
        }
    }
    int _216 = _213 ? (_68 ? _199 : _199) : _62;
    if (!_213)
    {
        for (int _191 = _216; _191 < 40; )
        {
            FragColor += _191;
            _191++;
            continue;
        }
        FragColor += _216;
    }
}

