// RUN: %dxc -T cs_6_6 %s | %FileCheck %s

// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap
// Make sure counter is marked in ResourceProperties.
// CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle {{.*}}, %dx.types.ResourceProperties { i32 36876, i32 12 })
// CHECK:call i32 @dx.op.bufferUpdateCounter

[numthreads(1, 1, 1)]
void main()
{
    AppendStructuredBuffer<uint3> buf = ResourceDescriptorHeap[0];
    buf.Append(uint3(1, 2, 3));
}