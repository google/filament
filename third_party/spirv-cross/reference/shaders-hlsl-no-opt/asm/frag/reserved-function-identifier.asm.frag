static float FragColor;

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

float _mat3(float a)
{
    return a + 1.0f;
}

float _RESERVED_IDENTIFIER_FIXUP_gl_Foo(int a)
{
    return float(a) + 1.0f;
}

void frag_main()
{
    float param = 2.0f;
    int param_1 = 4;
    FragColor = _mat3(param) + _RESERVED_IDENTIFIER_FIXUP_gl_Foo(param_1);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
