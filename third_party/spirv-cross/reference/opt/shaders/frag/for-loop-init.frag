#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out mediump int FragColor;

void main()
{
    do
    {
        FragColor = 16;
        for (mediump int _143 = 0; _143 < 25; )
        {
            FragColor += 10;
            _143++;
            continue;
        }
        for (mediump int _144 = 1; _144 < 30; )
        {
            FragColor += 11;
            _144++;
            continue;
        }
        mediump int _145;
        _145 = 0;
        for (; _145 < 20; )
        {
            FragColor += 12;
            _145++;
            continue;
        }
        mediump int _62 = _145 + 3;
        FragColor += _62;
        if (_62 == 40)
        {
            for (mediump int _149 = 0; _149 < 40; )
            {
                FragColor += 13;
                _149++;
                continue;
            }
            break;
        }
        FragColor += _62;
        mediump ivec2 _146;
        _146 = ivec2(0);
        for (; _146.x < 10; )
        {
            FragColor += _146.y;
            mediump ivec2 _142 = _146;
            _142.x = _146.x + 4;
            _146 = _142;
            continue;
        }
        for (mediump int _148 = _62; _148 < 40; )
        {
            FragColor += _148;
            _148++;
            continue;
        }
        FragColor += _62;
        break;
    } while(false);
}

