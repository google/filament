// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: object's templated type must have at least one element

struct Empty {};
Buffer<Empty> eb;


[ numthreads( 64, 2, 2 ) ]
void main( uint GI : SV_GroupIndex)
{
        eb[GI];
}