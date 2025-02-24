// RUN: %dxc -enable-16bit-types -Emain -Tcs_6_3 %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=U0:2:10i0;.0;0;0. | %FileCheck %s

// Check that the expected PIX UAV read-tracking is emitted (the atomicBinOp "|= 1") followed by the expected raw read:

// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle
// CHECK: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle
// CHECK: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle
// CHECK: call %dx.types.ResRet.i16 @dx.op.rawBufferLoad.i16

// Now the writes with atomicBinOp "|=2":

// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle
// CHECK: call void @dx.op.rawBufferStore.f32
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle
// CHECK: call void @dx.op.rawBufferStore.i32
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle
// CHECK: call void @dx.op.rawBufferStore.f16
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle
// CHECK: call void @dx.op.rawBufferStore.i16

struct S
{
    float4 f4;
    int4 i4;
    half hf;
    min16int hi;
};

RWStructuredBuffer<S> structuredUAV: register(u0);

[RootSignature(
    "DescriptorTable(UAV(u0, numDescriptors = 1, space = 0, offset = DESCRIPTOR_RANGE_OFFSET_APPEND))"
)]
[numthreads(1, 1, 1)]
void main()
{
    S s = structuredUAV[0];
    s.f4 += float4(1, 1, 1, 1);
    s.i4 += int4(1, 1, 1, 1);
    s.hi += 2;
    s.hf += 3.;
    structuredUAV[0] = s;
}