// RUN: not %dxc -T cs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

RWStructuredBuffer<uint> g_output : register(u1, space0);
RWTexture2D<uint> g_rwtexture2d : register(u1, space3);
[numthreads(64, 1, 1)] void main(int threadId : SV_DispatchThreadID)
{
  // The second argument for Load must be a variable where the function can
  // write the operation result Status.
  //
  // CHECK: 11:24: error: an lvalue argument should be used for returning the operation status
  g_output[threadId] = g_rwtexture2d.Load(0, 0);
}
