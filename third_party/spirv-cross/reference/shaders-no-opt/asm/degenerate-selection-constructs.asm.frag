#version 320 es
precision mediump float;
precision highp int;

layout(binding = 1, std140) uniform buf1
{
    highp vec2 resolution;
} _12;

layout(binding = 0, std140) uniform buf0
{
    highp vec2 injectionSwitch;
} _17;

layout(location = 0) out highp vec4 _GLF_color;

bool checkSwap(highp float a, highp float b)
{
    bool _33 = gl_FragCoord.y < (_12.resolution.y / 2.0);
    highp float _38;
    if (_33)
    {
        _38 = a;
    }
    else
    {
        highp float _355 = 0.0;
        _38 = _355;
    }
    bool _35;
    do
    {
        highp float _39;
        if (_33)
        {
            _39 = b;
        }
        else
        {
            highp float _360 = 0.0;
            _39 = _360;
        }
        if (_33)
        {
            _35 = _38 > _39;
        }
        if (true)
        {
            break;
        }
        else
        {
            break;
        }
    } while(false);
    highp float _42;
    if (_33)
    {
        highp float _367 = 0.0;
        _42 = _367;
    }
    else
    {
        _42 = a;
    }
    highp float _43;
    if (_33)
    {
        highp float _372 = 0.0;
        _43 = _372;
    }
    else
    {
        _43 = b;
    }
    if (!_33)
    {
        _35 = _42 < _43;
    }
    return _35;
}

void main()
{
    highp float data[10];
    for (int i = 0; i < 10; i++)
    {
        data[i] = float(10 - i) * _17.injectionSwitch.y;
    }
    for (int i_1 = 0; i_1 < 9; i_1++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (j < (i_1 + 1))
            {
                continue;
            }
            highp float param = data[i_1];
            highp float param_1 = data[j];
            bool doSwap = checkSwap(param, param_1);
            if (doSwap)
            {
                highp float temp = data[i_1];
                data[i_1] = data[j];
                data[j] = temp;
            }
        }
    }
    if (gl_FragCoord.x < (_12.resolution.x / 2.0))
    {
        _GLF_color = vec4(data[0] / 10.0, data[5] / 10.0, data[9] / 10.0, 1.0);
    }
    else
    {
        _GLF_color = vec4(data[5] / 10.0, data[9] / 10.0, data[0] / 10.0, 1.0);
    }
}

