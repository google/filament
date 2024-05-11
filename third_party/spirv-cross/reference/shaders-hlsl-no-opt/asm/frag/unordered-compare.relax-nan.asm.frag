static float4 A;
static float4 B;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float4 A : TEXCOORD0;
    float4 B : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

float4 test_vector()
{
    bool4 le = bool4(A.x < B.x, A.y < B.y, A.z < B.z, A.w < B.w);
    bool4 leq = bool4(A.x <= B.x, A.y <= B.y, A.z <= B.z, A.w <= B.w);
    bool4 ge = bool4(A.x > B.x, A.y > B.y, A.z > B.z, A.w > B.w);
    bool4 geq = bool4(A.x >= B.x, A.y >= B.y, A.z >= B.z, A.w >= B.w);
    bool4 eq = bool4(A.x == B.x, A.y == B.y, A.z == B.z, A.w == B.w);
    bool4 neq = bool4(A.x != B.x, A.y != B.y, A.z != B.z, A.w != B.w);
    neq = bool4(A.x != B.x, A.y != B.y, A.z != B.z, A.w != B.w);
    return ((((float4(le) + float4(leq)) + float4(ge)) + float4(geq)) + float4(eq)) + float4(neq);
}

float test_scalar()
{
    bool le = A.x < B.x;
    bool leq = A.x <= B.x;
    bool ge = A.x > B.x;
    bool geq = A.x >= B.x;
    bool eq = A.x == B.x;
    bool neq = A.x != B.x;
    return ((((float(le) + float(leq)) + float(ge)) + float(geq)) + float(eq)) + float(neq);
}

void frag_main()
{
    FragColor = test_vector() + test_scalar().xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    A = stage_input.A;
    B = stage_input.B;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
