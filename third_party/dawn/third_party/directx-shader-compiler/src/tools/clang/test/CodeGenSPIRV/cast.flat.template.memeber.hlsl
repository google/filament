// RUN: %dxc -T cs_6_8 -E main %s -spirv | FileCheck %s

struct T
{
    float3 direction;
};

template<class TT>
struct S
{
    TT L;
};

RWStructuredBuffer< S<T> > o;

// CHECK: [[f120:%.+]] = OpConstant %float 120
// CHECK: [[v120:%.+]] = OpConstantComposite %v3float [[f120]] [[f120]] [[f120]]
// CHECK: [[T:%.+]] = OpConstantComposite %T [[v120]]
// CHECK: [[S:%.+]] = OpConstantComposite %S [[T]]

[numthreads(32, 32, 1)]
void main(uint32_t threadID : SV_DispatchThreadID)
{
    uint32_t infinity = 0x78;
    S<T> s = (S<T>)infinity;

// CHECK: OpStore {{%.*}} [[S]]
    o[0] = s;
}
