// RUN: %dxc -EMain -Tps_6_6 %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=S0:1:1i1;.256;512;1024. | %FileCheck %s

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

// Check we wrote sampler data to the PIX UAV. We told the pass to output starting at offset 512.
// Add 8 to skip the "out of bounds" record. Add 4 to point to the "read" field within the next entry = 524
// CHECK: call void @dx.op.bufferStore.i32(
// CHECK:i32 524, i32 undef, i32 16777216

// twice: 512 + 8 + 8*3+4 = 548
// CHECK: call void @dx.op.bufferStore.i32(
// CHECK:i32 548, i32 undef, i32 16777216
