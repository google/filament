Texture2DMS<float4> TextureF_MS : register(t0);

// VS_ResolveColor2D: not needed, we reuse VS_Passthrough2D

float4 PS_ResolveColor2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint width, height, samples;
    TextureF_MS.GetDimensions(width, height, samples);
    uint2 coord = uint2(inTexCoord.x * float(width), inTexCoord.y * float(height));
    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    for (uint s = 0; s < samples; s++)
    {
        color += TextureF_MS.Load(coord, s).rgba;
    }
    return color / float(samples);

    // Potential performance improvement: We could remove the TextureF_MS.GetDimensions() call
    // and pass width, height, and invSamples via constants to the shader. This would allow us
    // to calculate the final texture coordinates in the vertex shader already and instead of
    // dividing by samples we could multiply by invSamples at the end.
    // But maybe graphics drivers even do these optimizations automatically after the texture
    // metrics are known at rendering time?
}
