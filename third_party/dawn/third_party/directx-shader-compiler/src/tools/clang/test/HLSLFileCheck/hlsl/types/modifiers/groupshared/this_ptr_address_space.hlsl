// RUN: %dxc -E main -T cs_6_2 %s | FileCheck %s

// Test that the address space of the this pointer is honored
// when accessing data members or calling member functions.
// Also a regression test for GitHub #1957

struct Foo { int x; int getX() { return x; } };
int i, j;

// CHECK: @[[gs:.*]] = addrspace(3) global [2 x i32] undef
groupshared Foo foo[2];

RWStructuredBuffer<int4> output;

[numthreads(8,8,1)]
void main( uint gidx : SV_GroupIndex )
{
  output[gidx] = int4(
    // getelementptr & addrspacecast constant expressions
    // CHECK: load i32, i32 addrspace(3)* getelementptr inbounds ([2 x i32], [2 x i32] addrspace(3)* @[[gs]], i32 0, i32 0)
    foo[0].x, 
    // CHECK: load i32, i32 addrspace(3)* getelementptr inbounds ([2 x i32], [2 x i32] addrspace(3)* @[[gs]], i32 0, i32 1)
    foo[1].getX(),

    // getelementptr & addrspacecast instructions
    // CHECK: %[[gep:.*]] = getelementptr [2 x i32], [2 x i32] addrspace(3)* @[[gs]], i32 0, i32 %{{.*}}
    // CHECK: load i32, i32 addrspace(3)* %[[gep]]
    foo[i].x,
    // CHECK: %[[gep:.*]] = getelementptr [2 x i32], [2 x i32] addrspace(3)* @[[gs]], i32 0, i32 %{{.*}}
    // CHECK: load i32, i32 addrspace(3)* %[[gep]]
    foo[j].getX()); 
}