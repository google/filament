// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

enum class Sample
{
    One = 0x0001,
};

void main()
{
    // CHECK: OpStore %s %int_1
    Sample s = (Sample)0x1u;
}