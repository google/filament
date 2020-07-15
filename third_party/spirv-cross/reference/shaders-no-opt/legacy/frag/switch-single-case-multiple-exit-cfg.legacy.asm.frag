#version 100
precision mediump float;
precision highp int;

vec2 _19;

void main()
{
    highp vec2 _30;
    for (int SPIRV_Cross_Dummy15 = 0; SPIRV_Cross_Dummy15 < 1; SPIRV_Cross_Dummy15++)
    {
        if (gl_FragCoord.x != gl_FragCoord.x)
        {
            _30 = _19;
            break;
        }
        highp vec2 _29 = _19;
        _29.y = _19.y;
        _30 = _29;
        break;
    }
    gl_FragData[0] = vec4(_30, 1.0, 1.0);
}

