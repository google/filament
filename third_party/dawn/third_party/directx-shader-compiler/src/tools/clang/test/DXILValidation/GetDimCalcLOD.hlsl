TextureCube<float4> cube;

SamplerState    g_sam;

float4 main(float2 uv : UV) : SV_TARGET
{
    uint w;
    uint h;

    cube.GetDimensions(w,h);
    float lod = cube.CalculateLevelOfDetail(g_sam, float3(uv,1));
    return float4(w, h, lod, 1.0);
}

