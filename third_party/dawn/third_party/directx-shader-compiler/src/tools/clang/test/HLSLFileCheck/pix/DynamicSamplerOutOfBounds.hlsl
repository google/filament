// RUN: %dxc -EMain -Tps_6_6 %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=S0:1:1i1;.256;512;520. | %FileCheck %s

static sampler sampler0 = SamplerDescriptorHeap[0];
static sampler sampler3 = SamplerDescriptorHeap[3];
Texture2D tx : register(t2);

float4 Main() : SV_Target
{
    float4 a = tx.Sample(sampler0, float2(0,0));
    float4 b = tx.Sample(sampler3, float2(0,0));
    return a + b;
}

// check it's 6.6:
// CHECK: call %dx.types.Handle @dx.op.createHandleFromBinding

// The large integers are encoded flags for the ResourceAccessStyle (an enumerated type in lib\DxilPIXPasses\DxilShaderAccessTracking.cpp) for this access

// The start of sampler records has been passed in as 512. The limit of the whole buffer is 520, leaving just one eight-byte record for the out-of-bounds record.
// There are therefore no expected in-bounds references to samplers, so any such reference should go to the out-of-bounds offset at 512:

// Out of bounds sampler access should be at offset 512
// CHECK: call void @dx.op.bufferStore.i32(
// CHECK:i32 512, i32 undef, i32 134217728
