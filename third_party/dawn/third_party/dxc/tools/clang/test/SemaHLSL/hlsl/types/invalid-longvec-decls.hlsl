// RUN: %dxc -T ps_6_9 -DTYPE=LongVec    -DNUM=5   -verify %s
// RUN: %dxc -T ps_6_9 -DTYPE=LongVecSub -DNUM=128 -verify %s
// RUN: %dxc -T ps_6_9                   -DNUM=1024 -verify %s

// Add tests for base types and instantiated template classes with longvecs
// Size of the vector shouldn't matter, but using a few different ones just in case.

#define PASTE_(x,y) x##y
#define PASTE(x,y) PASTE_(x,y)

#ifndef TYPE
#define TYPE LongVecTpl<NUM>
#endif

struct LongVec {
  float4 f;
  vector<float,NUM> vec;
};

struct LongVecSub : LongVec {
  int3 is;
};

template <int N>
struct LongVecTpl {
  float4 f;
  vector<float,N> vec;
};

vector<float, NUM> global_vec; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
vector<float, NUM> global_vec_arr[10]; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
TYPE global_vec_rec; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
TYPE global_vec_rec_arr[10]; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}

cbuffer BadBuffy {
  vector<float, NUM> cb_vec; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
  vector<float, NUM> cb_vec_arr[10]; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
  TYPE cb_vec_rec; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
  TYPE cb_vec_rec_arr[10]; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
};

tbuffer BadTuffy {
  vector<float, NUM> tb_vec; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
  vector<float, NUM> tb_vec_arr[10]; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
  TYPE tb_vec_rec; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
  TYPE tb_vec_rec_arr[10]; // expected-error{{vectors of over 4 elements in cbuffers or tbuffers are not supported}}
};

ConstantBuffer< TYPE > const_buf; // expected-error{{vectors of over 4 elements in ConstantBuffers or TextureBuffers are not supported}}
TextureBuffer< TYPE > tex_buf; // expected-error{{vectors of over 4 elements in ConstantBuffers or TextureBuffers are not supported}}

[shader("pixel")]
vector<float, NUM> main( // expected-error{{vectors of over 4 elements in entry function return type are not supported}}
                     vector<float, NUM> vec : V) : SV_Target { // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
  return vec;
}

[shader("vertex")]
TYPE vs_main( // expected-error{{vectors of over 4 elements in entry function return type are not supported}}
                     TYPE parm : P) : SV_Target { // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
  parm.f = 0;
  return parm;
}


[shader("geometry")]
[maxvertexcount(3)]
void gs_point(line TYPE e, // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
              inout PointStream<TYPE> OutputStream0) {} // expected-error{{vectors of over 4 elements in geometry streams are not supported}}

[shader("geometry")]
[maxvertexcount(12)]
void gs_line(line TYPE a, // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
             inout LineStream<TYPE> OutputStream0) {} // expected-error{{vectors of over 4 elements in geometry streams are not supported}}


[shader("geometry")]
[maxvertexcount(12)]
void gs_line(line TYPE a, // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
             inout TriangleStream<TYPE> OutputStream0) {} // expected-error{{vectors of over 4 elements in geometry streams are not supported}}

[shader("domain")]
[domain("tri")]
void ds_main(OutputPatch<TYPE, 3> TrianglePatch) {} // expected-error{{vectors of over 4 elements in tessellation patches are not supported}}

void patch_const(InputPatch<TYPE, 3> inpatch, // expected-error{{vectors of over 4 elements in tessellation patches are not supported}}
			   OutputPatch<TYPE, 3> outpatch) {} // expected-error{{vectors of over 4 elements in tessellation patches are not supported}}

[shader("hull")]
[domain("tri")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(32)]
[patchconstantfunc("patch_const")]
void hs_main(InputPatch<TYPE, 3> TrianglePatch) {} // expected-error{{vectors of over 4 elements in tessellation patches are not supported}}

RaytracingAccelerationStructure RTAS;

struct [raypayload] DXRLongVec {
  float4 f : write(closesthit) : read(caller);
  vector<float,NUM> vec : write(closesthit) : read(caller);
};

struct [raypayload] DXRLongVecSub : DXRLongVec {
  int3 is : write(closesthit) : read(caller);
};

template<int N>
struct [raypayload] DXRLongVecTpl {
  float4 f : write(closesthit) : read(caller);
  vector<float,N> vec : write(closesthit) : read(caller);
};

#define RTTYPE PASTE(DXR,TYPE)

[shader("raygeneration")]
void raygen() {
  RTTYPE p = (RTTYPE)0;
  RayDesc ray = (RayDesc)0;
  TraceRay(RTAS, RAY_FLAG_NONE, 0, 0, 1, 0, ray, p); // expected-error{{vectors of over 4 elements in user-defined struct parameter are not supported}}
  CallShader(0, p); // expected-error{{vectors of over 4 elements in user-defined struct parameter are not supported}}
}


[shader("closesthit")]
void closesthit(inout RTTYPE payload, // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
		in RTTYPE attribs ) { // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
  RayDesc ray;
  TraceRay( RTAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload ); // expected-error{{vectors of over 4 elements in user-defined struct parameter are not supported}}
  CallShader(0, payload); // expected-error{{vectors of over 4 elements in user-defined struct parameter are not supported}}
}

[shader("anyhit")]
void AnyHit( inout RTTYPE payload, // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
	      in RTTYPE attribs  ) // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
{
}

[shader("miss")]
void Miss(inout RTTYPE payload){ // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
  RayDesc ray;
  TraceRay( RTAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload ); // expected-error{{vectors of over 4 elements in user-defined struct parameter are not supported}}
  CallShader(0, payload); // expected-error{{vectors of over 4 elements in user-defined struct parameter are not supported}}
}

[shader("intersection")]
void Intersection() {
  float hitT = RayTCurrent();
  RTTYPE attr = (RTTYPE)0;
  bool bReported = ReportHit(hitT, 0, attr); // expected-error{{vectors of over 4 elements in attributes are not supported}}
}

[shader("callable")]
void callable1(inout RTTYPE p) { // expected-error{{vectors of over 4 elements in entry function parameters are not supported}}
  CallShader(0, p); // expected-error{{vectors of over 4 elements in user-defined struct parameter are not supported}}
}

groupshared LongVec as_pld;

[shader("amplification")]
[numthreads(1,1,1)]
void Amp() {
  DispatchMesh(1,1,1,as_pld); // expected-error{{vectors of over 4 elements in user-defined struct parameter are not supported}}
}

struct NodeLongVec {
  uint3 grid : SV_DispatchGrid;
  vector<float,NUM> vec;
};

struct NodeLongVecSub : NodeLongVec {
  int3 is;
};

template<int N>
struct NodeLongVecTpl {
  uint3 grid : SV_DispatchGrid;
  vector<float,N> vec;
};

#define NTYPE PASTE(Node,TYPE)

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeMaxDispatchGrid(8,1,1)]
void broadcast(DispatchNodeInputRecord<NTYPE> input,  // expected-error{{vectors of over 4 elements in node records are not supported}}
                NodeOutput<TYPE> output) // expected-error{{vectors of over 4 elements in node records are not supported}}
{
  ThreadNodeOutputRecords<TYPE> touts; // expected-error{{vectors of over 4 elements in node records are not supported}}
  GroupNodeOutputRecords<TYPE> gouts; // expected-error{{vectors of over 4 elements in node records are not supported}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(8,1,1)]
void coalesce(GroupNodeInputRecords<TYPE> input) {} // expected-error{{vectors of over 4 elements in node records are not supported}}

[Shader("node")]
[NodeLaunch("thread")]
void threader(ThreadNodeInputRecord<TYPE> input) {} // expected-error{{vectors of over 4 elements in node records are not supported}}
