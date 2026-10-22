// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

//Make sure support case memcpy has multiple overloads.

// CHECK: @main

struct S
{
	float3 f;
	uint i;
};

StructuredBuffer<S> b0;
RWStructuredBuffer<S> u0;

groupshared S s;


[numthreads(64,1,1)]
void main(uint id : SV_GroupThreadID) {
 if (id==0) {
   s = b0[id];
   s.i += 2;
 }
 S s0 = b0[id];
 s0.i += id;

 u0[id] = s0;
 u0[id*2] = b0[id*2];
}