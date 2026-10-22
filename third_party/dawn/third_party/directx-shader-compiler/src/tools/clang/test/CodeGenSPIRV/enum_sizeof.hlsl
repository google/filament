// RUN: %dxc -T cs_6_0 -E main  -fcgl %s -spirv | FileCheck %s

enum E1 : uint64_t
{
    v1 = 0,
};

enum E2 : uint32_t
{
    v2 = 0,
};

struct S {
  E1 e1;
  E2 e2;
};

RWBuffer<int> b;

[numthreads(128, 1, 1)]
void main()
{
// CHECK: OpImageWrite {{%.*}} %uint_0 %int_8 None
    b[0] = sizeof(E1);

// CHECK: OpImageWrite {{%.*}} %uint_1 %int_4 None
    b[1] = sizeof(E2);

// CHECK: OpImageWrite {{%.*}} %uint_2 %int_16 None
    b[2] = sizeof(S);
}
