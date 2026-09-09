// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

[[vk::constant_id(0)]] const uint kConst = 1;

RWTexture2D<float3> img;

[numthreads(1, 1, 1)]
void main(uint3 svDispatchThreadId : SV_DISPATCHTHREADID)
{
	// CHECK: OpSpecConstantComposite %v2uint %kConst %kConst
    img[svDispatchThreadId.xy * kConst] = 0;
}
