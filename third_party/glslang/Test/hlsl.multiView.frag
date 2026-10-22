
float4 main(uint ViewIndex : SV_ViewID)
{
    return float4(ViewIndex, 0.0f, 0.0f, 0.0f);
}
