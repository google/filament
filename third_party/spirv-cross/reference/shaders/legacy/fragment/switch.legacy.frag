#version 100
precision mediump float;
precision highp int;

varying highp float vIndexF;

void main()
{
    int vIndex = int(vIndexF);
    highp vec4 v = vec4(0.0);
    for (int SPIRV_Cross_Dummy21 = 0; SPIRV_Cross_Dummy21 < 1; SPIRV_Cross_Dummy21++)
    {
        if (vIndex == 2)
        {
            v = vec4(0.0, 2.0, 3.0, 4.0);
            break;
        }
        else if ((vIndex == 4) || (vIndex == 5))
        {
            v = vec4(1.0, 2.0, 3.0, 4.0);
            break;
        }
        else if ((vIndex == 8) || (vIndex == 9))
        {
            v = vec4(40.0, 20.0, 30.0, 40.0);
            break;
        }
        else if (vIndex == 10)
        {
            v = vec4(10.0);
            highp vec4 _43 = v;
            highp vec4 _44 = vec4(1.0);
            highp vec4 _45 = _43 + _44;
            v = _45;
            highp vec4 _46 = v;
            highp vec4 _47 = vec4(2.0);
            highp vec4 _48 = _46 + _47;
            v = _48;
            break;
        }
        else if (vIndex == 11)
        {
            highp vec4 _43 = v;
            highp vec4 _44 = vec4(1.0);
            highp vec4 _45 = _43 + _44;
            v = _45;
            highp vec4 _46 = v;
            highp vec4 _47 = vec4(2.0);
            highp vec4 _48 = _46 + _47;
            v = _48;
            break;
        }
        else if (vIndex == 12)
        {
            highp vec4 _46 = v;
            highp vec4 _47 = vec4(2.0);
            highp vec4 _48 = _46 + _47;
            v = _48;
            break;
        }
        else
        {
            v = vec4(10.0, 20.0, 30.0, 40.0);
            break;
        }
    }
    highp vec4 w = vec4(20.0);
    for (int SPIRV_Cross_Dummy165 = 0; SPIRV_Cross_Dummy165 < 1; SPIRV_Cross_Dummy165++)
    {
        if ((vIndex == 10) || (vIndex == 20))
        {
            w = vec4(40.0);
            break;
        }
    }
    gl_FragData[0] = v + w;
}

