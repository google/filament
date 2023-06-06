#version 100
precision mediump float;
precision highp int;

varying highp float vIndexF;

void main()
{
    int _13 = int(vIndexF);
    highp vec4 _65;
    highp vec4 _66;
    highp vec4 _68;
    for (int spvDummy25 = 0; spvDummy25 < 1; spvDummy25++)
    {
        if (_13 == 2)
        {
            _68 = vec4(0.0, 2.0, 3.0, 4.0);
            break;
        }
        else if ((_13 == 4) || (_13 == 5))
        {
            _68 = vec4(1.0, 2.0, 3.0, 4.0);
            break;
        }
        else if ((_13 == 8) || (_13 == 9))
        {
            _68 = vec4(40.0, 20.0, 30.0, 40.0);
            break;
        }
        else if (_13 == 10)
        {
            _65 = vec4(10.0);
            highp vec4 _45 = _65 + vec4(1.0);
            _66 = _45;
            highp vec4 _48 = _66 + vec4(2.0);
            _68 = _48;
            break;
        }
        else if (_13 == 11)
        {
            _65 = vec4(0.0);
            highp vec4 _45 = _65 + vec4(1.0);
            _66 = _45;
            highp vec4 _48 = _66 + vec4(2.0);
            _68 = _48;
            break;
        }
        else if (_13 == 12)
        {
            _66 = vec4(0.0);
            highp vec4 _48 = _66 + vec4(2.0);
            _68 = _48;
            break;
        }
        else
        {
            _68 = vec4(10.0, 20.0, 30.0, 40.0);
            break;
        }
    }
    highp vec4 _70;
    for (int spvDummy146 = 0; spvDummy146 < 1; spvDummy146++)
    {
        if ((_13 == 10) || (_13 == 20))
        {
            _70 = vec4(40.0);
            break;
        }
        else
        {
            _70 = vec4(20.0);
            break;
        }
    }
    gl_FragData[0] = _68 + _70;
}

