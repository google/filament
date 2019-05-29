#version 450
#extension GL_GOOGLE_cpp_style_line_directive : require

layout(location = 0) out float FragColor;
layout(location = 0) in float vColor;

#line 8 "test.frag"
void main()
{
    float _80;
#line 8 "test.frag"
    FragColor = 1.0;
#line 9 "test.frag"
    FragColor = 2.0;
#line 10 "test.frag"
    _80 = vColor;
    if (_80 < 0.0)
    {
#line 12 "test.frag"
        FragColor = 3.0;
    }
    else
    {
#line 16 "test.frag"
        FragColor = 4.0;
    }
    for (int _126 = 0; float(_126) < (40.0 + _80); )
    {
#line 21 "test.frag"
        FragColor += 0.20000000298023223876953125;
#line 22 "test.frag"
        FragColor += 0.300000011920928955078125;
        _126 += (int(_80) + 5);
        continue;
    }
    switch (int(_80))
    {
        case 0:
        {
#line 28 "test.frag"
            FragColor += 0.20000000298023223876953125;
#line 29 "test.frag"
            break;
        }
        case 1:
        {
#line 32 "test.frag"
            FragColor += 0.4000000059604644775390625;
#line 33 "test.frag"
            break;
        }
        default:
        {
#line 36 "test.frag"
            FragColor += 0.800000011920928955078125;
#line 37 "test.frag"
            break;
        }
    }
    for (;;)
    {
        FragColor += (10.0 + _80);
#line 43 "test.frag"
        if (FragColor < 100.0)
        {
        }
        else
        {
            break;
        }
    }
}

