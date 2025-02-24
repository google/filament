// RUN: %dxc -T ps_6_0  %s | FileCheck %s

// Make sure only set nonUniformIndex when mark it.
// CHECK:call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 %{{.*}}, i1 false)
// CHECK:call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 1, i32 %{{.*}}, i1 true)

Buffer<float> Tuniform[16];
Buffer<float> Tnonuniform[16];

float main(int i : IN) : SV_Target
{
    if (i >= 8)
    {
        return Tuniform[i].Load(0);
    }
    else
    {
        return Tnonuniform[NonUniformResourceIndex(i)].Load(0);
    }
}