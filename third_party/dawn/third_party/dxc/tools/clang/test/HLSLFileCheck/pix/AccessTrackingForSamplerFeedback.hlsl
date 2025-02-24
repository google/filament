// RUN: %dxc -Emain -Tcs_6_5 %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=M0:0:1i0;S0:1:1i0;U0:2:10i0;.0;0;0. | %FileCheck %s

// Check we added the UAV:                                                                      v----metadata position: not important for this check
// CHECK:  %PIX_ShaderAccessUAV_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 [[S:[0-9]+]], i32 0, i1 false)

// Feedback UAV:
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle, i32 28

// Texture:
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle, i32 12

// Sampler:
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_ShaderAccessUAV_Handle, i32 0

Texture2D texture : register(t0);
SamplerState samplerState : register(s0);

FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP> map : register(u0);

[numthreads(4, 4, 1)] 
void main(uint3 threadId : SV_DispatchThreadId) {
  float2 uv = threadId.xy;
  uv /= 256;

  map.WriteSamplerFeedbackLevel(texture, samplerState, uv, threadId.x % 8);
}
