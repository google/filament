cbuffer Registers
{
    float registers_foo : packoffset(c0);
};


static float FragColor;

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

float add_value(float v, float w)
{
    return v + w;
}

void frag_main()
{
    FragColor = add_value(10.0f, registers_foo);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
