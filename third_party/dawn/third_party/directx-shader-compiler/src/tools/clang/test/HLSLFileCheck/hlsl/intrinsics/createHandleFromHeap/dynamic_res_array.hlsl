// RUN: %dxc -T cs_6_6 %s | %FileCheck %s

// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 0, i1 false, i1 false)
// CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle {{.*}}, %dx.types.ResourceProperties { i32 4098, i32 1033 })

struct A
{
    RWTexture2D<float4> tex_array[3];
};

A create()
{
    A a;
    a.tex_array[0] = ResourceDescriptorHeap[0];
    a.tex_array[1] = ResourceDescriptorHeap[1];
    a.tex_array[2] = ResourceDescriptorHeap[2];

    return a;
}

static const A a = create();

[numthreads(64, 1, 1)]
void main(
    uint3 groupID       : SV_GroupID,
    uint3 dispatchID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
    a.tex_array[0][groupThreadID.xy] = 1;
}