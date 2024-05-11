#version 320 es
precision mediump float;
precision highp int;

layout(binding = 1, std140) uniform buf1
{
    highp vec2 resolution;
} _9;

layout(binding = 0, std140) uniform buf0
{
    highp vec2 injectionSwitch;
} _13;

layout(location = 0) out highp vec4 _GLF_color;

bool checkSwap(highp float a, highp float b)
{
    bool _153 = gl_FragCoord.y < (_9.resolution.y / 2.0);
    highp float _160;
    if (_153)
    {
        _160 = a;
    }
    else
    {
        highp float _159 = 0.0;
        _160 = _159;
    }
    bool _147;
    do
    {
        highp float _168;
        if (_153)
        {
            _168 = b;
        }
        else
        {
            highp float _167 = 0.0;
            _168 = _167;
        }
        if (_153)
        {
            _147 = _160 > _168;
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
    highp float _180;
    if (_153)
    {
        highp float _179 = 0.0;
        _180 = _179;
    }
    else
    {
        _180 = a;
    }
    highp float _186;
    if (_153)
    {
        highp float _185 = 0.0;
        _186 = _185;
    }
    else
    {
        _186 = b;
    }
    if (!_153)
    {
        _147 = _180 < _186;
    }
    return _147;
}

void main()
{
    highp float data[10];
    for (int i = 0; i < 10; i++)
    {
        data[i] = float(10 - i) * _13.injectionSwitch.y;
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
    if (gl_FragCoord.x < (_9.resolution.x / 2.0))
    {
        _GLF_color = vec4(data[0] / 10.0, data[5] / 10.0, data[9] / 10.0, 1.0);
    }
    else
    {
        _GLF_color = vec4(data[5] / 10.0, data[9] / 10.0, data[0] / 10.0, 1.0);
    }
}

