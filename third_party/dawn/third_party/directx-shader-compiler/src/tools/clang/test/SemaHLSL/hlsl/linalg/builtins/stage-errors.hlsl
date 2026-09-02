// RUN: %dxc -T lib_6_10 -DMATRIX_COPY_CONVERT %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_FILL %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_GET_COORDINATE %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_GET_ELEMENT %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_MULTIPLY %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_MULTIPLY_ACCUMULATE %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_SET_ELEMENT %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_STORE_TO_DESCRIPTOR %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_LENGTH %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_ACCUMULATE %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_LOAD_FROM_MEMORY %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_STORE_TO_MEMORY %s -verify
// RUN: %dxc -T lib_6_10 -DMATRIX_ACCUMULATE_TO_MEMORY %s -verify

RWByteAddressBuffer buf;
groupshared float gs_arr[64];

void CallFunction()
{
  // ComponentType::I1, 5x4, Use::Accumulator, Scope::ThreadGroup
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(1, 5, 4, 2, 2)]] mat1;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(1, 5, 4, 2, 2)]] mat2;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(1, 5, 4, 2, 2)]] mat3;
  float4 vecA = {0.0, 1.0, 2.0, 3.0};
  float4 vecB = {4.0, 5.0, 6.0, 7.0};
  float4 vecC = {8.0, 9.0, 0.0, 1.0};

#ifdef MATRIX_COPY_CONVERT
  #define DO_FUNC __builtin_LinAlg_CopyConvertMatrix(mat2, mat1, true);
#endif

#ifdef MATRIX_FILL
  #define DO_FUNC __builtin_LinAlg_FillMatrix(mat1, 15);
#endif

#ifdef MATRIX_GET_COORDINATE
  #define DO_FUNC uint2 coord = __builtin_LinAlg_MatrixGetCoordinate(mat1, 0);
#endif

#ifdef MATRIX_GET_ELEMENT
  float elem;
  #define DO_FUNC __builtin_LinAlg_MatrixGetElement(elem, mat1, 0);
#endif

#ifdef MATRIX_MULTIPLY
  #define DO_FUNC __builtin_LinAlg_MatrixMatrixMultiply(mat1, mat2, mat3);
#endif

#ifdef MATRIX_MULTIPLY_ACCUMULATE
  #define DO_FUNC __builtin_LinAlg_MatrixMatrixMultiplyAccumulate(mat1, mat2, mat3, mat1);
#endif

#ifdef MATRIX_SET_ELEMENT
  #define DO_FUNC __builtin_LinAlg_MatrixSetElement(mat2, mat1, 1, 1);
#endif

#ifdef MATRIX_STORE_TO_DESCRIPTOR
  #define DO_FUNC __builtin_LinAlg_MatrixStoreToDescriptor(mat1, buf, 1, 2, 3, 4);
#endif

#ifdef MATRIX_LENGTH
  #define DO_FUNC __builtin_LinAlg_MatrixLength(mat1);
#endif

#ifdef MATRIX_ACCUMULATE
  #define DO_FUNC __builtin_LinAlg_MatrixAccumulate(mat1, mat2, mat3);
#endif

#ifdef MATRIX_LOAD_FROM_MEMORY
  #define DO_FUNC __builtin_LinAlg_MatrixLoadFromMemory(mat1, gs_arr, 0, 0, 0);
#endif

#ifdef MATRIX_STORE_TO_MEMORY
  #define DO_FUNC __builtin_LinAlg_MatrixStoreToMemory(mat1, gs_arr, 0, 0, 0);
#endif

#ifdef MATRIX_ACCUMULATE_TO_MEMORY
  #define DO_FUNC __builtin_LinAlg_MatrixAccumulateToMemory(mat1, gs_arr, 0, 0, 0, 0);
#endif

  // The builtins below are allowed in all stages, if they raise an error
  // then the test will fail with "saw unexpected diagnostic"
  uint layout = __builtin_LinAlg_MatrixQueryAccumulatorLayout();
  __builtin_LinAlg_MatrixLoadFromDescriptor(mat1, buf, 5, 5, 5, 4);
  __builtin_LinAlg_MatrixOuterProduct(mat1, vecA, vecB);
  __builtin_LinAlg_MatrixAccumulateToDescriptor(mat1, buf, 1, 2, 3, 4);
  __builtin_LinAlg_MatrixVectorMultiply(vecA, mat1, true, vecB, 1);
  __builtin_LinAlg_MatrixVectorMultiplyAdd(vecA, mat1, true, vecB, 2, vecC);
  int4 outVec;
  __builtin_LinAlg_Convert(outVec, vecA, 1, 2);
  __builtin_LinAlg_VectorAccumulateToDescriptor(buf, 0, 64, vecA);

  // expected-error@+12{{builtin unavailable in shader stage 'pixel' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+11{{builtin unavailable in shader stage 'vertex' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+10{{builtin unavailable in shader stage 'node' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+9{{builtin unavailable in shader stage 'raygeneration' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+8{{builtin unavailable in shader stage 'intersection' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+7{{builtin unavailable in shader stage 'callable' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+6{{builtin unavailable in shader stage 'anyhit' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+5{{builtin unavailable in shader stage 'closesthit' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+4{{builtin unavailable in shader stage 'miss' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+3{{builtin unavailable in shader stage 'hull' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+2{{builtin unavailable in shader stage 'domain' (requires 'compute', 'mesh' or 'amplification')}}
  // expected-error@+1{{builtin unavailable in shader stage 'geometry' (requires 'compute', 'mesh' or 'amplification')}}
  DO_FUNC
}

// --- Allowed Stages ---

[shader("compute")]
[numthreads(4,4,4)]
void mainCS(uint ix : SV_GroupIndex, uint3 id : SV_GroupThreadID) {
  CallFunction();
}

struct Verts {
    float4 position : SV_Position;
};

[shader("mesh")]
[NumThreads(8, 8, 2)]
[OutputTopology("triangle")]
void mainMeS(out vertices Verts verts[32], uint ix : SV_GroupIndex) {
  CallFunction();
  SetMeshOutputCounts(32, 16);
  Verts v = {0.0, 0.0, 0.0, 0.0};
  verts[ix] = v;
}

struct AmpPayload {
    float2 dummy;
};

[numthreads(8, 1, 1)]
[shader("amplification")]
void mainAS()
{
    CallFunction();
    AmpPayload pld;
    pld.dummy = float2(1.0,2.0);
    DispatchMesh(8, 1, 1, pld);
}

// --- Prohibited Stages ---

[shader("pixel")]
// expected-note@+1{{entry function defined here}}
float4 mainPS(uint ix : SV_PrimitiveID) : SV_TARGET {
  CallFunction();
  return 1.0;
}

[shader("vertex")]
// expected-note@+1{{entry function defined here}}
float4 mainVS(uint ix : SV_VertexID) : OUT {
  CallFunction();
  return 1.0;
}

[shader("node")]
[nodedispatchgrid(8,1,1)]
[numthreads(64,2,2)]
// expected-note@+1{{entry function defined here}}
void mainNS() {
  CallFunction();
}

[shader("raygeneration")]
// expected-note@+1{{entry function defined here}}
void mainRG() {
  CallFunction();
}

[shader("intersection")]
// expected-note@+1{{entry function defined here}}
void mainIS() {
  CallFunction();
}

struct Attribs { float2 barys; };

[shader("callable")]
// expected-note@+1{{entry function defined here}}
void mainCALL(inout Attribs attrs) {
  CallFunction();
}

struct [raypayload] RayPayload
{
    float elem
          : write(caller,closesthit,anyhit,miss)
          : read(caller,closesthit,anyhit,miss);
};

[shader("anyhit")]
// expected-note@+1{{entry function defined here}}
void mainAH(inout RayPayload pld, in Attribs attrs) {
 CallFunction();
}

[shader("closesthit")]
// expected-note@+1{{entry function defined here}}
void mainCH(inout RayPayload pld, in Attribs attrs) {
  CallFunction();
}

[shader("miss")]
// expected-note@+1{{entry function defined here}}
void mainMS(inout RayPayload pld) {
  CallFunction();
}

struct PosStruct {
  float4 pos : SV_Position;
};

struct PCStruct
{
  float Edges[3]  : SV_TessFactor;
  float Inside : SV_InsideTessFactor;
  float4 test : TEST;
};

PCStruct HSPatch(InputPatch<PosStruct, 3> ip,
                 OutputPatch<PosStruct, 3> op,
                 uint ix : SV_PrimitiveID)
{
  PCStruct a;
  a.Edges[0] = ip[0].pos.w;
  a.Edges[1] = ip[0].pos.w;
  a.Edges[2] = ip[0].pos.w;
  a.Inside = ip[0].pos.w;
  return a;
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatch")]
// expected-note@+1{{entry function defined here}}
PosStruct mainHS(InputPatch<PosStruct, 3> p, uint ix : SV_OutputControlPointID)
{
  CallFunction();
  PosStruct s;
  s.pos = p[ix].pos;
  return s;
}

[shader("domain")]
[domain("tri")]
// expected-note@+1{{entry function defined here}}
PosStruct mainDS(const OutputPatch<PosStruct, 3> patch,
                 uint ix : SV_PrimitiveID)
{
  CallFunction();
  PosStruct v;
  v.pos = patch[0].pos;
  return v;
}

float4 a;

[shader("geometry")]
[maxvertexcount(1)]
// expected-note@+1{{entry function defined here}}
void mainGS(triangle float4 array[3] : SV_Position, uint ix : SV_GSInstanceID,
            inout PointStream<PosStruct> OutputStream)
{
  CallFunction();
  PosStruct s;
  s.pos = a;
  OutputStream.Append(s);
  OutputStream.RestartStrip();
}
