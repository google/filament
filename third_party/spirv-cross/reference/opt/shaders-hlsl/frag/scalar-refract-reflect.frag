static float FragColor;
static float3 vRefract;

struct SPIRV_Cross_Input
{
    float3 vRefract : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

float SPIRV_Cross_Reflect(float i, float n)
{
    return i - 2.0 * dot(n, i) * n;
}

float SPIRV_Cross_Refract(float i, float n, float eta)
{
    float NoI = n * i;
    float NoI2 = NoI * NoI;
    float k = 1.0 - eta * eta * (1.0 - NoI2);
    if (k < 0.0)
    {
        return 0.0;
    }
    else
    {
        return eta * i - (eta * NoI + sqrt(k)) * n;
    }
}

void frag_main()
{
    FragColor = SPIRV_Cross_Refract(vRefract.x, vRefract.y, vRefract.z);
    FragColor += SPIRV_Cross_Reflect(vRefract.x, vRefract.y);
    FragColor += refract(vRefract.xy, vRefract.yz, vRefract.z).y;
    FragColor += reflect(vRefract.xy, vRefract.zy).y;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vRefract = stage_input.vRefract;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
