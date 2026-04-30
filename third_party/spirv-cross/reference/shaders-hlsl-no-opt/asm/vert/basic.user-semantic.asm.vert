static float4 gl_Position;
static float4 in_var_B;
static float4 out_var_A;

struct SPIRV_Cross_Input
{
    float4 in_var_B : B;
};

struct SPIRV_Cross_Output
{
    float4 out_var_A : A;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = 1.0f.xxxx;
    out_var_A = in_var_B;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    in_var_B = stage_input.in_var_B;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.out_var_A = out_var_A;
    return stage_output;
}
