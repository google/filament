#version 450
#extension GL_GOOGLE_cpp_style_line_directive : require

layout(location = 0) in vec3 iv;
layout(location = 0) out vec3 ov;

void main()
{
#line 137 "test.frag"
#line 106 "test.frag"
    bool _298 = iv.x < 0.0;
    if (_298)
    {
#line 107 "test.frag"
        ov.x = 50.0;
    }
    else
    {
#line 109 "test.frag"
        ov.x = 60.0;
    }
#line 114 "test.frag"
    for (int _529 = 0; _529 < 4; _529++)
    {
#line 106 "test.frag"
        if (_298)
        {
#line 107 "test.frag"
            ov.x = 50.0;
        }
        else
        {
#line 109 "test.frag"
            ov.x = 60.0;
        }
#line 117 "test.frag"
        if (iv.y < 0.0)
        {
#line 118 "test.frag"
            ov.y = 70.0;
        }
        else
        {
#line 120 "test.frag"
            ov.y = 80.0;
        }
    }
#line 126 "test.frag"
    for (int _533 = 0; _533 < 4; _533++)
    {
#line 106 "test.frag"
        if (_298)
        {
#line 107 "test.frag"
            ov.x = 50.0;
        }
        else
        {
#line 109 "test.frag"
            ov.x = 60.0;
        }
#line 114 "test.frag"
        for (int _537 = 0; _537 < 4; _537++)
        {
#line 106 "test.frag"
            if (_298)
            {
#line 107 "test.frag"
                ov.x = 50.0;
            }
            else
            {
#line 109 "test.frag"
                ov.x = 60.0;
            }
#line 117 "test.frag"
            if (iv.y < 0.0)
            {
#line 118 "test.frag"
                ov.y = 70.0;
            }
            else
            {
#line 120 "test.frag"
                ov.y = 80.0;
            }
        }
#line 130 "test.frag"
        if (iv.z < 0.0)
        {
#line 131 "test.frag"
            ov.z = 100.0;
        }
        else
        {
#line 133 "test.frag"
            ov.z = 120.0;
        }
    }
#line 142 "test.frag"
}

