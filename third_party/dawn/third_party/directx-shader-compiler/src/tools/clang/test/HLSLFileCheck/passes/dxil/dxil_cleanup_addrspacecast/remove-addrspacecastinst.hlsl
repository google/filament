// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: @main()
// CHECK-NOT: addrspacecast
// CHECK: ret void

struct Foo { int x; int getX() { return x; } };
groupshared Foo foo[2];
RWStructuredBuffer<int> output;
int i;
[numthreads(1,1,1)]
void main() { output[i] =  foo[i].getX(); }
