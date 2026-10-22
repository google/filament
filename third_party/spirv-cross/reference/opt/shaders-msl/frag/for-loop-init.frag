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
    do
    {
        out.FragColor = 16;
        for (int _143 = 0; _143 < 25; )
        {
            out.FragColor += 10;
            _143++;
            continue;
        }
        for (int _144 = 1; _144 < 30; )
        {
            out.FragColor += 11;
            _144++;
            continue;
        }
        int _145;
        _145 = 0;
        for (; _145 < 20; )
        {
            out.FragColor += 12;
            _145++;
            continue;
        }
        int _62 = _145 + 3;
        out.FragColor += _62;
        if (_62 == 40)
        {
            for (int _149 = 0; _149 < 40; )
            {
                out.FragColor += 13;
                _149++;
                continue;
            }
            break;
        }
        out.FragColor += _62;
        int2 _146;
        _146 = int2(0);
        for (; _146.x < 10; )
        {
            out.FragColor += _146.y;
            int2 _142 = _146;
            _142.x = _146.x + 4;
            _146 = _142;
            continue;
        }
        for (int _148 = _62; _148 < 40; )
        {
            out.FragColor += _148;
            _148++;
            continue;
        }
        out.FragColor += _62;
        break;
    } while(false);
    return out;
}

