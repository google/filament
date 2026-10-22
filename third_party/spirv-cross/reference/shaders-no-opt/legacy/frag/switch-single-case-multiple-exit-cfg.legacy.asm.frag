#version 100
precision mediump float;
precision highp int;

vec2 _52;

void main()
{
    highp vec2 _53;
    for (int spvDummy15 = 0; spvDummy15 < 1; spvDummy15++)
    {
        if (gl_FragCoord.x != gl_FragCoord.x)
        {
            _53 = _52;
            break;
        }
        highp vec2 _51;
        _51.y = _52.y;
        _53 = _51;
        break;
    }
    gl_FragData[0] = vec4(_53, 1.0, 1.0);
}

