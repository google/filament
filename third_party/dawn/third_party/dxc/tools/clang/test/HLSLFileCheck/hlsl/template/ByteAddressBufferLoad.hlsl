// RUN: %dxc -E CSMain -T cs_6_6 -HV 2021 -fcgl %s | FileCheck %s
template<typename T>
struct MyStructA
{
    T m_0;
};

struct MyStructB
{
    MyStructA<float> m_a;
    float m_1;
    float m_2;
    float m_3;
};

ByteAddressBuffer g_bab;
RWBuffer<float> result;

// This test verifies that templates can be used both as the argument to
// ByteAddressBuffer::Load and as a member of a structure passed as an argument
// to ByteAddressLoad as long as the specialized template conforms to the rules
// for HLSL (must only contain integral and floating point members).
// CHECK-NOT: error

[numthreads(1,1,1)]
void CSMain()
{
  // CHECK: call %"struct.MyStructA<float>"* @"dx.hl.op.ro.%\22struct.MyStructA<float>\22* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]+}}, %dx.types.Handle %{{[0-9]+}}, i32 0)
  MyStructA<float> a = g_bab.Load<MyStructA<float> >(0);
  result[0] = a.m_0;

  // CHECK: call %struct.MyStructB* @"dx.hl.op.ro.%struct.MyStructB* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]+}}, %dx.types.Handle %{{[0-9]+}}, i32 1)
  MyStructB b = g_bab.Load<MyStructB>(1);
  result[1] = b.m_a.m_0;
}
