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

_4 _26;
int _31;

layout(location = 0) flat in int _2;
layout(location = 0) out int _3;

void main()
{
    bool _76 = false;
    do
    {
        _5 _33;
        _33._m0 = 0;
        _33._m1 = 10;
        _4 _41;
        _41 = _26;
        int _45;
        _4 _42;
        bool _79;
        int _44 = 0;
        for (;;)
        {
            if (_33._m0 < _33._m1)
            {
                int _34 = _33._m0;
                int _35 = _33._m0 + int(1u);
                _33._m0 = _35;
                _42 = _4(1u, _34);
            }
            else
            {
                _4 _65 = _41;
                _65._m0 = 0u;
                _42 = _65;
            }
            bool _55_ladder_break = false;
            switch (int(_42._m0))
            {
                case 0:
                {
                    _3 = _44;
                    _76 = true;
                    _79 = true;
                    _55_ladder_break = true;
                    break;
                }
                default:
                {
                    _79 = false;
                    _55_ladder_break = true;
                    break;
                }
                case 1:
                {
                    break;
                }
            }
            if (_55_ladder_break)
            {
                break;
            }
            _45 = _44 + _2;
            _41 = _42;
            _44 = _45;
            continue;
        }
        if (_79)
        {
            break;
        }
        break;
    } while(false);
}

