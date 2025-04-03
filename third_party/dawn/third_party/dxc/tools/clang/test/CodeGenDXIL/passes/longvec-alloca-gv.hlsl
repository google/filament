// RUN: %dxc -fcgl -T lib_6_9 %s | FileCheck %s

// Mainly a source for the ScalarReductionOfAggregatesHLSL(SROA)
//  and DynamicIndexingVectorToArray(DIVA) IR tests with native vectors
//  using allocas, static globals, and parameters.
// Dynamically accessed 1-element vectors should get skipped by SROA,
//  but addressed by DynamicIndexingVectorToArray (hence the name).
// Larger vectors should be untouched.
// Arrays of vectors get some special treatment as well.
// Verifies that the original code is as expected for the IR tests.

struct VectRec1 {
  float1 f : REC1;
};
struct VectRec2 {
  float2 f : REC2;
};

// Vec2s will be preserved.
// CHECK-DAG: @dyglob2 = internal global <2 x float> zeroinitializer, align 4
// CHECK-DAG: @dygar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
// CHECK-DAG: @dygrec2 = internal global %struct.VectRec2 zeroinitializer, align 4

// Dynamic vec1s will get replaced with dynamic vector to array.
// CHECK-DAG: @dyglob1 = internal global <1 x float> zeroinitializer, align 4
// CHECK-DAG: @dygar1 = internal global [2 x <1 x float>] zeroinitializer, align 4
// CHECK-DAG: @dygrec1 = internal global %struct.VectRec1 zeroinitializer, align 4

// Vec2s will be preserved.
// CHECK-DAG: @stglob2 = internal global <2 x float> zeroinitializer, align 4
// CHECK-DAG: @stgar2 = internal global [3 x <2 x float>] zeroinitializer, align 4
// CHECK-DAG: @stgrec2 = internal global %struct.VectRec2 zeroinitializer, align 4

// Static vec1s will get replaced with SROA.
// CHECK-DAG: @stglob1 = internal global <1 x float> zeroinitializer, align 4
// CHECK-DAG: @stgar1 = internal global [2 x <1 x float>] zeroinitializer, align 4
// CHECK-DAG: @stgrec1 = internal global %struct.VectRec1 zeroinitializer, align 4

static float1 dyglob1;
static float2 dyglob2;
static float1 dygar1[2];
static float2 dygar2[3];
static VectRec1 dygrec1;
static VectRec2 dygrec2;

static float1 stglob1;
static float2 stglob2;
static float1 stgar1[2];
static float2 stgar2[3];
static VectRec1 stgrec1;
static VectRec2 stgrec2;

// Test assignment operators.
// Vec2s should be skipped by SROA and DIVA
// DIVA will lower statically-indexed vectors and vectors in an array.
// CHECK-LABEL: define <4 x float> @"\01?tester
export float4 tester(int ix : IX, float vals[12] : VAL) {

  // Vec2s will be preserved.
  // CHECK-DAG: %dyloc2 = alloca <2 x float>, align 4
  // CHECK-DAG: %dylar2 = alloca [4 x <2 x float>], align 4
  // CHECK-DAG: %dylorc2 = alloca %struct.VectRec2, align 4

  // Dynamic local vec1s will get replaced with dynamic vector to array.
  // CHECK-DAG: %dyloc1 = alloca <1 x float>, align 4
  // CHECK-DAG: %dylar1 = alloca [3 x <1 x float>], align 4
  // CHECK-DAG: %dylorc1 = alloca %struct.VectRec1, align 4

  // Vec2s will be preserved.
  // CHECK-DAG: %stloc2 = alloca <2 x float>, align 4
  // CHECK-DAG: %stlar2 = alloca [4 x <2 x float>], align 4
  // CHECK-DAG: %stlorc2 = alloca %struct.VectRec2, align 4

  // Static local vec1s will get replaced by various passes.
  // CHECK-DAG: %stloc1 = alloca <1 x float>, align 4
  // CHECK-DAG: %stlar1 = alloca [3 x <1 x float>], align 4
  // CHECK-DAG: %stlorc1 = alloca %struct.VectRec1, align 4

  float1 dyloc1;
  float2 dyloc2;
  float1 dylar1[3];
  float2 dylar2[4];
  VectRec1 dylorc1;
  VectRec2 dylorc2;

  float1 stloc1;
  float2 stloc2;
  float1 stlar1[3];
  float2 stlar2[4];
  VectRec1 stlorc1;
  VectRec2 stlorc2;

  if (ix > 0) {
    stloc1[0] = dyloc1[ix] = vals[0];
    stloc2[1] = dyloc2[ix] = vals[1];
    stlar1[1][0] = dylar1[ix][ix] = vals[2];
    stlar2[1][0] = dylar2[ix][ix] = vals[3];
    stlorc1.f[0] = dylorc1.f[ix] = vals[4];
    stlorc2.f[1] = dylorc2.f[ix] = vals[5];

    stglob1[0] = dyglob1[ix] = vals[6];
    stglob2[1] = dyglob2[ix] = vals[7];
    stgar1[1][0] = dygar1[ix][ix] = vals[8];
    stgar2[1][1] = dygar2[ix][ix] = vals[9];
    stgrec1.f[0] = dygrec1.f[ix] = vals[10];
    stgrec2.f[1] = dygrec2.f[ix] = vals[11];
  }
  return float4(dyloc1.x, dyloc2.y, stloc1.x, stloc2.y) + float4(dylar1[ix][ix], dylar2[ix][ix], stlar1[0].x, stlar2[0].y) +
  float4(dyglob1.x, dyglob2.y, stglob1.x, stglob2.y) + float4(dygar1[ix][ix], dygar2[ix][ix], stgar1[0].x, stgar2[0].y) +
    float4(stlorc1.f, stlorc2.f[1], dylorc1.f, dylorc2.f[ix]) + float4(stgrec1.f, stgrec2.f[1], dygrec1.f, dygrec2.f[ix]);
}

