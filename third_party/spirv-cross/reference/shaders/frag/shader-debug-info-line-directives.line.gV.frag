#version 450
#extension GL_GOOGLE_cpp_style_line_directive : require

layout(location = 0) in vec3 iv;
layout(location = 0) out vec3 ov;

void func0()
{
#line 104 "test.frag"
#line 106 "test.frag"
    if (iv.x < 0.0)
    {
#line 107 "test.frag"
        ov.x = 50.0;
    }
    else
    {
#line 109 "test.frag"
        ov.x = 60.0;
    }
#line 110 "test.frag"
}

void func1()
{
#line 112 "test.frag"
#line 114 "test.frag"
    for (int i = 0; i < 4; i++)
    {
#line 116 "test.frag"
        func0();
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
#line 122 "test.frag"
}

void func2()
{
#line 124 "test.frag"
#line 126 "test.frag"
    for (int i = 0; i < 4; i++)
    {
#line 128 "test.frag"
        func0();
#line 129 "test.frag"
        func1();
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
#line 135 "test.frag"
}

void main()
{
#line 137 "test.frag"
#line 139 "test.frag"
    func0();
#line 140 "test.frag"
    func1();
#line 141 "test.frag"
    func2();
#line 142 "test.frag"
}

