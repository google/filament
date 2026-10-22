// RUN: %dxc -Zi -E main -T as_6_5 %s | FileCheck %s

// CHECK: call void @llvm.dbg.value(metadata %struct.smallPayload{{.*}}*

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

struct smallPayload
{
    float f1;
    float2 f2;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.f1 = 1;
    p.f2 = float2(2,3);
    DispatchMesh(1, 1, 1, p);
}