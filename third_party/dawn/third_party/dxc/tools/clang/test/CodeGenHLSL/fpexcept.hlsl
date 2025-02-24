// RUN: %dxc -E main -T ps_6_0 %s

float3 main() : SV_Target
{
    // Some calls known to cause floating point exceptions when evaluated
    return float3(pow(0.0f, 0.0f), pow(-1.0f, 0.5f), pow(0.0f, -1.0f));
}