#version 450

struct _4
{
    uint _m0;
    int _m1;
};

struct _5
{
    int _m0;
    int _m1;
};

layout(location = 0) flat in int _2;
layout(location = 0) out int _3;

_4 _16;
int _21;

void main()
{
    bool _25 = false;
    do
    {
        _5 _26;
        _26._m0 = 0;
        _26._m1 = 10;
        _4 _35;
        _35 = _16;
        int _39;
        _4 _36;
        bool _59;
        int _38 = 0;
        for (;;)
        {
            if (_26._m0 < _26._m1)
            {
                int _27 = _26._m0;
                int _28 = _26._m0 + int(1u);
                _26._m0 = _28;
                _36 = _4(1u, _27);
            }
            else
            {
                _4 _48 = _35;
                _48._m0 = 0u;
                _36 = _48;
            }
            bool _45_ladder_break = false;
            switch (int(_36._m0))
            {
                case 0:
                {
                    _3 = _38;
                    _25 = true;
                    _59 = true;
                    _45_ladder_break = true;
                    break;
                }
                default:
                {
                    _59 = false;
                    _45_ladder_break = true;
                    break;
                }
                case 1:
                {
                    break;
                }
            }
            if (_45_ladder_break)
            {
                break;
            }
            _39 = _38 + _2;
            _35 = _36;
            _38 = _39;
            continue;
        }
        if (_59)
        {
            break;
        }
        break;
    } while(false);
}

