#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant int2 _184 = {};
constant int _199 = {};

struct main0_out
{
    int FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = 16;
    for (int _168 = 0; _168 < 25; )
    {
        out.FragColor += 10;
        _168++;
        continue;
    }
    for (int _169 = 1; _169 < 30; )
    {
        out.FragColor += 11;
        _169++;
        continue;
    }
    int _170;
    _170 = 0;
    for (; _170 < 20; )
    {
        out.FragColor += 12;
        _170++;
        continue;
    }
    int _62 = _170 + 3;
    out.FragColor += _62;
    bool _68 = _62 == 40;
    if (_68)
    {
        for (int _171 = 0; _171 < 40; )
        {
            out.FragColor += 13;
            _171++;
            continue;
        }
    }
    else
    {
        out.FragColor += _62;
    }
    bool2 _211 = bool2(_68);
    int2 _212 = int2(_211.x ? _184.x : _184.x, _211.y ? _184.y : _184.y);
    bool _213 = _68 ? true : false;
    bool2 _214 = bool2(_213);
    if (!_213)
    {
        int2 _177;
        _177 = int2(_214.x ? _212.x : int2(0).x, _214.y ? _212.y : int2(0).y);
        for (; _177.x < 10; )
        {
            out.FragColor += _177.y;
            int2 _167 = _177;
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
            out.FragColor += _191;
            _191++;
            continue;
        }
        out.FragColor += _216;
    }
    return out;
}

