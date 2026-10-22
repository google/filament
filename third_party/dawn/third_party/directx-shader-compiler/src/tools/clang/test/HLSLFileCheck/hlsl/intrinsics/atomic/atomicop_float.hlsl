// RUN: %dxc -T cs_6_6 %s | FileCheck %s

groupshared float   resG[256];
RWBuffer<float>     resB;
RWStructuredBuffer<float> resS;

[numthreads(1,1,1)]
void main( float a : A, int b: B, float c :C)
{
  // Test some disallowed atomic binop intrinsics with floats as both args

  // CHECK: error: no matching function for call to 'InterlockedAdd'
  // CHECK: error: no matching function for call to 'InterlockedMin'
  // CHECK: error: no matching function for call to 'InterlockedMax'
  // CHECK: error: no matching function for call to 'InterlockedAnd'
  // CHECK: error: no matching function for call to 'InterlockedOr'
  // CHECK: error: no matching function for call to 'InterlockedXor'
  InterlockedAdd(resG[0], a);
  InterlockedMin(resG[0], a);
  InterlockedMax(resG[0], a);
  InterlockedAnd(resG[0], a);
  InterlockedOr(resG[0], a);
  InterlockedXor(resG[0], a);

  // CHECK: error: no matching function for call to 'InterlockedAdd'
  // CHECK: error: no matching function for call to 'InterlockedMin'
  // CHECK: error: no matching function for call to 'InterlockedMax'
  // CHECK: error: no matching function for call to 'InterlockedAnd'
  // CHECK: error: no matching function for call to 'InterlockedOr'
  // CHECK: error: no matching function for call to 'InterlockedXor'
  InterlockedAdd(resB[0], a);
  InterlockedMin(resB[0], a);
  InterlockedMax(resB[0], a);
  InterlockedAnd(resB[0], a);
  InterlockedOr(resB[0], a);
  InterlockedXor(resB[0], a);

  // CHECK: error: no matching function for call to 'InterlockedAdd'
  // CHECK: error: no matching function for call to 'InterlockedMin'
  // CHECK: error: no matching function for call to 'InterlockedMax'
  // CHECK: error: no matching function for call to 'InterlockedAnd'
  // CHECK: error: no matching function for call to 'InterlockedOr'
  // CHECK: error: no matching function for call to 'InterlockedXor'
  InterlockedAdd(resS[0], a);
  InterlockedMin(resS[0], a);
  InterlockedMax(resS[0], a);
  InterlockedAnd(resS[0], a);
  InterlockedOr(resS[0], a);
  InterlockedXor(resS[0], a);

  // Try the same with an integer second arg to make sure they still fail

  // CHECK: error: no matching function for call to 'InterlockedAdd'
  // CHECK: error: no matching function for call to 'InterlockedMin'
  // CHECK: error: no matching function for call to 'InterlockedMax'
  // CHECK: error: no matching function for call to 'InterlockedAnd'
  // CHECK: error: no matching function for call to 'InterlockedOr'
  // CHECK: error: no matching function for call to 'InterlockedXor'
  InterlockedAdd(resG[0], b);
  InterlockedMin(resG[0], b);
  InterlockedMax(resG[0], b);
  InterlockedAnd(resG[0], b);
  InterlockedOr(resG[0], b);
  InterlockedXor(resG[0], b);

  // CHECK: error: no matching function for call to 'InterlockedAdd'
  // CHECK: error: no matching function for call to 'InterlockedMin'
  // CHECK: error: no matching function for call to 'InterlockedMax'
  // CHECK: error: no matching function for call to 'InterlockedAnd'
  // CHECK: error: no matching function for call to 'InterlockedOr'
  // CHECK: error: no matching function for call to 'InterlockedXor'
  InterlockedAdd(resB[0], b);
  InterlockedMin(resB[0], b);
  InterlockedMax(resB[0], b);
  InterlockedAnd(resB[0], b);
  InterlockedOr(resB[0], b);
  InterlockedXor(resB[0], b);

  // CHECK: error: no matching function for call to 'InterlockedAdd'
  // CHECK: error: no matching function for call to 'InterlockedMin'
  // CHECK: error: no matching function for call to 'InterlockedMax'
  // CHECK: error: no matching function for call to 'InterlockedAnd'
  // CHECK: error: no matching function for call to 'InterlockedOr'
  // CHECK: error: no matching function for call to 'InterlockedXor'
  InterlockedAdd(resS[0], b);
  InterlockedMin(resS[0], b);
  InterlockedMax(resS[0], b);
  InterlockedAnd(resS[0], b);
  InterlockedOr(resS[0], b);
  InterlockedXor(resS[0], b);
}

