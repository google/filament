// RUN: %dxc -T cs_6_6 -E main %s -spirv 2>&1 | FileCheck %s

RWStructuredBuffer<uint> Buf1;
RWStructuredBuffer<uint> Buf2;

struct A
{
  RWStructuredBuffer<uint> Buffer;
  void Increment() { Buffer.IncrementCounter(); }
};

struct B : A {};
struct C : B {};

[numthreads(64, 1, 1)]
void main()
{
  B b;
  b.Buffer = Buf1;

  C c;
  c.Buffer = Buf2;

  // CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_Buf1 %uint_0
  // CHECK: OpAtomicIAdd %int [[ac]] %uint_1 %uint_0 %int_1
  b.Increment();

  // CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_Buf2 %uint_0
  // CHECK: OpAtomicIAdd %int [[ac]] %uint_1 %uint_0 %int_1 
  c.Increment();
}
