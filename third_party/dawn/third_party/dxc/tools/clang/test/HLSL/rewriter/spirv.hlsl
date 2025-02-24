
struct S
{
	int4 a;
	int4 b;
};

RWStructuredBuffer<S> structuredUAV : register(u0);

RWBuffer<int4> outBufferUAV : register(u1);

[RootSignature("UAV(u0, numDescriptors=2, space=0, offset=DESCRIPTOR_RANGE_OFFSET_APPEND)")]
[numthreads(1, 1, 1)]
void main(uint id : SV_DispatchThreadID) {
	outBufferUAV[id] = structuredUAV[id].a + structuredUAV[id].b;
}