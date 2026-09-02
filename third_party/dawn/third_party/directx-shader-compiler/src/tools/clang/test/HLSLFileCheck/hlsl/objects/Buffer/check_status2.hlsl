// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure no checkaccess is generated.
// CHECK-NOT: dx.op.checkAccessFullyMapped.i32(i32 71

RWBuffer<float4> g_buffer;
RWBuffer<uint> g_result;

[numthreads(1, 1, 1)]
void main()
{
    uint res;
    float4 data = g_buffer.Load(0, res);
    g_result[0] = 1;
}