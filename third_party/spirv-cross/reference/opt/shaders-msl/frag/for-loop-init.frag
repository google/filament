#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    int FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    int _145;
    for (;;)
    {
        out.FragColor = 16;
        _145 = 0;
        for (; _145 < 25; )
        {
            out.FragColor += 10;
            _145++;
            continue;
        }
        for (int _146 = 1; _146 < 30; )
        {
            out.FragColor += 11;
            _146++;
            continue;
        }
        int _147;
        _147 = 0;
        for (; _147 < 20; )
        {
            out.FragColor += 12;
            _147++;
            continue;
        }
        int _62 = _147 + 3;
        out.FragColor += _62;
        if (_62 == 40)
        {
            for (int _151 = 0; _151 < 40; )
            {
                out.FragColor += 13;
                _151++;
                continue;
            }
            break;
        }
        out.FragColor += _62;
        int2 _148;
        _148 = int2(0);
        for (; _148.x < 10; )
        {
            out.FragColor += _148.y;
            int2 _144 = _148;
            _144.x = _148.x + 4;
            _148 = _144;
            continue;
        }
        for (int _150 = _62; _150 < 40; )
        {
            out.FragColor += _150;
            _150++;
            continue;
        }
        out.FragColor += _62;
        break;
    }
    return out;
}

