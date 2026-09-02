// RUN: %dxc -T cs_6_9 -DFTYPE=float2 %s | FileCheck %s
// RUN: %dxc -T cs_6_9 -DFTYPE=half2 -enable-16bit-types %s | FileCheck %s


// https://github.com/microsoft/DirectXShaderCompiler/issues/7915
// Test long vector casting between uint2 and float2
// which would crash as reported by a user.

// CHECK: call %dx.types.Handle @dx.op.createHandleFromBinding
// CHECK: fptoui
// CHECK: uitofp
// CHECK: fptoui
// CHECK: uitofp
// CHECK: call void @dx.op.rawBufferVectorStore

RWStructuredBuffer<FTYPE> input;
RWStructuredBuffer<FTYPE> output;

FTYPE f1(uint2 p) { return p; }
uint2 f2(FTYPE p) { return f1(p); }

[numthreads(1,1,1)]
void main()
{
  output[0] = f2(input[0]);
}
