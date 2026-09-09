// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that (nested) empty structs compile away

// CHECK: define void @main()
// CHECK-NOT: %{{.*}} =
// CHECK: ret void

struct EmptyStruct {};
struct OuterStruct { EmptyStruct empty; };

OuterStruct global;
static OuterStruct staticGlobal;
cbuffer SomeCBuffer { OuterStruct cbufferField; };
ConstantBuffer<OuterStruct> cb;
StructuredBuffer<OuterStruct> sb;

OuterStruct main(OuterStruct input,
    out OuterStruct output)
{
    OuterStruct local = input;
    staticGlobal = global;
    output = cbufferField;
    return local;
}