cbuffer UBO : register(b0)
{
    float4 _18_a[2] : packoffset(c0);
    float4 _18_b : packoffset(c2);
    float4 _18_c : packoffset(c3);
    row_major float4x4 _18_d : packoffset(c4);
    float _18_e : packoffset(c8);
    float2 _18_f : packoffset(c8.z);
    float _18_g : packoffset(c9);
    float2 _18_h : packoffset(c9.z);
    float _18_i : packoffset(c10);
    float2 _18_j : packoffset(c10.z);
    float _18_k : packoffset(c11);
    float2 _18_l : packoffset(c11.z);
    float _18_m : packoffset(c12);
    float _18_n : packoffset(c12.y);
    float _18_o : packoffset(c12.z);
    float4 _18_p : packoffset(c13);
    float4 _18_q : packoffset(c14);
    float3 _18_r : packoffset(c15);
    float4 _18_s : packoffset(c16);
    float4 _18_t : packoffset(c17);
    float4 _18_u : packoffset(c18);
    float _18_v : packoffset(c19);
    float _18_w : packoffset(c19.y);
    float _18_x : packoffset(c19.z);
    float _18_y : packoffset(c19.w);
    float _18_z : packoffset(c20);
    float _18_aa : packoffset(c20.y);
    float _18_ab : packoffset(c20.z);
    float _18_ac : packoffset(c20.w);
    float _18_ad : packoffset(c21);
    float _18_ae : packoffset(c21.y);
    float4 _18_ef : packoffset(c22);
};


static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = _18_a[1];
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
