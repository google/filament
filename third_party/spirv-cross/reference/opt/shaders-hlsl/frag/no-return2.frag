static float4 vColor;

struct SPIRV_Cross_Input
{
    float4 vColor : TEXCOORD0;
};

void frag_main()
{
}

void main(SPIRV_Cross_Input stage_input)
{
    vColor = stage_input.vColor;
    frag_main();
}
