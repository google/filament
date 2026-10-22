// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// A regression test for sub-memcpys
// These might get eliminated if their attempted replacement removes them.
// The goal is to create a memcpy that fails to RAUW the first two levels
// but then finds itself able to for the last memcpy
// If handled improperly, the result returned will be undefined and fail validation

// CHECK: @main
// CHECK-NOT: memcpy
// CHECK: fmul
// CHECK-NOT: float undef
// CHECK: phi float
// CHECK: ret void
struct OuterStruct
{
  float fval;
  struct InnerStruct {
    float val1;
    float val2;
  } Array[3];
};

cbuffer cbuf : register(b1)
{
 OuterStruct g_oStruct[1];
};

float main(int doit : A) : SV_Target
{
  OuterStruct oStruct;

  // Need a conditional so the memcpy source won't dominate the output
  for (; doit >= 0; --doit) {
    uint multiplier = 4;
    // Because the struct is copied within conditional block, the source will be
    // a GEP in the if statement which won't dominate the usage, thwarting the first RAUW replacement
    // Copying twice thwarts the first RAUW
    oStruct = g_oStruct[doit];
    oStruct = g_oStruct[doit];
    // Must use the struct twice to thwart the second RAUW replacement
    // At this stage, trivial reusage is enough.
    multiplier = oStruct.Array[0].val2 + oStruct.Array[0].val1;

    // If memcpy is wrong, undef floats will be part of this calculation
    oStruct.fval *= multiplier;
  }

  // This makes use of the memory that should be copied as part of the sub-memcpy
  return oStruct.fval;
}
