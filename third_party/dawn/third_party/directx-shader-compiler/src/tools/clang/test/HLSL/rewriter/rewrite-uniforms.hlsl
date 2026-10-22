#define RS "RootFlags(0),DescriptorTable(UAV(u0, numDescriptors = 1), CBV(b0, numDescriptors = 1))"

[RootSignature(RS)]
[numthreads(4,8,16)]
void FloatFunc(uint3 id : SV_DispatchThreadID, uniform RWStructuredBuffer<float> buf, uniform uint ui)
{
    buf[id.x+id.y+id.z] = id.x;
}

[RootSignature(RS)]
[numthreads(4,8,16)]
void IntFunc(uint3 id : SV_DispatchThreadID, uniform RWStructuredBuffer<int> buf, uniform uint ui)
{
    buf[id.x+id.y+id.z] = id.x + ui;
}
