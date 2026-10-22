// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// A test for different lowermemcpy approaches at different levels.
// The original memcpy is to copy the entire struct.
// If only used once, the src location can replace the dest location
// But by being used twice, this isn't possible.
// The next memcpy which is meant to copy the array is similarly thwarted by double use
// The last level, which is a memcpy for the struct array elements has only one usage
// so it seems able to use RAUW. However, this memcpy came from the splitting of the
// previous memcopies. The src and dst are GEPs fashioned expressly to copy just this
// portion of the aggragate to satisfy whatever requirements might come later.
// So src and dst don't seem to have any other users than the memcpy they were created for.
// When that memcpy is deleted, thinking it has been replaced, these GEPs are left without
// users and promptly removed. In fact, they do have users, but it is not apparent becuase
// at this stage, the user only references the original complete aggregate alloca.
// when mem2reg tries to convert the regions of memory that these would have populated
// it determines they are empty and makes them undefined (0),
// which ultimately removes much of the code.

// Very similar to memcpy_split_replace, but this one produces validation errors when unfixed


// CHECK: @main
// CHECK-NOT: memcpy
// CHECK: fmul
// CHECK-NOT: float undef
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
 OuterStruct g_oStruct[4];
};

float main(int doit : A) : SV_Target
{
  OuterStruct oStruct;

  {
    float multiplier = 4.0;
    // At this stage, even trivial double usage will thwart RAUW at the top level
    // Neither conditional nor +1 are needed, but make a more credible usage pattern
    oStruct = g_oStruct[doit];
    if (doit)
      oStruct = g_oStruct[doit+1];
    // Double usage of the array element thwarts it at the second level too.
    // However, each scalar member is used just once, allowing for RAUW
    multiplier = oStruct.Array[1].val1 +
      oStruct.Array[1].val2;

    // If memcpy is wrong, undef floats will be part of this calculation
    oStruct.fval *= multiplier;
  }

  return oStruct.fval;
}
