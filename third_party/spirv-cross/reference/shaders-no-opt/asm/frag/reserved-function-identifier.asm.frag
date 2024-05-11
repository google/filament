#version 450

layout(location = 0) out float FragColor;

float _mat3(float a)
{
    return a + 1.0;
}

float _RESERVED_IDENTIFIER_FIXUP_gl_Foo(int a)
{
    return float(a) + 1.0;
}

void main()
{
    float param = 2.0;
    int param_1 = 4;
    FragColor = _mat3(param) + _RESERVED_IDENTIFIER_FIXUP_gl_Foo(param_1);
}

