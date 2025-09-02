// RUN: %dxc -T lib_6_9 -DTYPE=HitStruct -verify %s
// RUN: %dxc -T lib_6_9 -DTYPE=HitStructSub -verify %s


#define PASTE_(x,y) x##y
#define PASTE(x,y) PASTE_(x,y)

#ifndef TYPE
#define TYPE HitTpl<dx::HitObject>
#endif

// Add tests for base types and instantiated template classes with HitObjects

struct HitStruct {
  float4 f;
  dx::HitObject hit;
};

struct HitStructSub : HitStruct {
  int3 is;
};

template <typename T>
struct HitTpl {
  float4 f;
  T val;
};

TYPE global_type;
// expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}
// expected-note@16{{'dx::HitObject' field declared here}}
dx::HitObject global_hit;
// expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}
dx::HitObject global_hit_arr[10];
// expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}

static TYPE static_gv;
// expected-error@-1{{object 'dx::HitObject' is not allowed in global variables}}
// expected-note@16{{'dx::HitObject' field declared here}}

cbuffer BadBuffy {
  dx::HitObject cb_hit;
  // expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}
  dx::HitObject cb_hit_arr[10];
  // expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}
};

tbuffer BadTuffy {
  dx::HitObject tb_vec; 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}
  dx::HitObject tb_vec_arr[10];
  // expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}
  TYPE tb_vec_rec; 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  TYPE tb_vec_rec_arr[10]; 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in cbuffers or tbuffers}}
  // expected-note@16{{'dx::HitObject' field declared here}}
};

StructuredBuffer<TYPE> struct_buf;
// expected-error@-1{{object 'dx::HitObject' is not allowed in structured buffers}}
// expected-note@16{{'dx::HitObject' field declared here}}
RWStructuredBuffer<TYPE> rw_struct_buf;
// expected-error@-1{{object 'dx::HitObject' is not allowed in structured buffers}}
// expected-note@16{{'dx::HitObject' field declared here}}
ConstantBuffer<TYPE> const_buf;
// expected-error@-1{{object 'dx::HitObject' is not allowed in ConstantBuffers or TextureBuffers}}
// expected-note@16{{'dx::HitObject' field declared here}}
TextureBuffer<TYPE> tex_buf;
// expected-error@-1{{object 'dx::HitObject' is not allowed in ConstantBuffers or TextureBuffers}}
// expected-note@16{{'dx::HitObject' field declared here}}

ByteAddressBuffer bab;
RWByteAddressBuffer rw_bab;

[Shader("raygeneration")]
void main()
{
  bab.Load<TYPE>(0);
  // expected-error@-1{{object 'dx::HitObject' is not allowed in builtin template parameters}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  // expected-error@-3{{Explicit template arguments on intrinsic Load must be a single numeric type}}
  rw_bab.Load<TYPE>(0);
  // expected-error@-1{{object 'dx::HitObject' is not allowed in builtin template parameters}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  // expected-error@-3{{Explicit template arguments on intrinsic Load must be a single numeric type}}
  TYPE val;
  rw_bab.Store<TYPE>(0, val);
  // expected-error@-1{{object 'dx::HitObject' is not allowed in builtin template parameters}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  // expected-error@-3{{Explicit template arguments on intrinsic Store must be a single numeric type}}
}

[shader("pixel")]
TYPE ps_main( 
// expected-error@-1{{object 'dx::HitObject' is not allowed in entry function return type}}
// expected-note@16{{'dx::HitObject' field declared here}}
    TYPE vec : V) : SV_Target {
    // expected-error@-1{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
  return vec;
}

[shader("vertex")]
TYPE vs_main(
// expected-error@-1{{object 'dx::HitObject' is not allowed in entry function return type}}
// expected-note@16{{'dx::HitObject' field declared here}}
    TYPE parm : P) : SV_Target {
    // expected-error@-1{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
  parm.f = 0;
  return parm;
}


[shader("geometry")]
[maxvertexcount(3)]
void gs_point(
    line TYPE e,
    // expected-error@-1{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
    inout PointStream<TYPE> OutputStream0)
    // expected-error@-1{{object 'dx::HitObject' is not allowed in geometry streams}}
    // expected-note@16{{'dx::HitObject' field declared here}}
{}

[shader("geometry")]
[maxvertexcount(12)]
void gs_line( 
    line TYPE a,
    // expected-error@-1{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
    inout LineStream<TYPE> OutputStream0)
    // expected-error@-1{{object 'dx::HitObject' is not allowed in geometry streams}}
    // expected-note@16{{'dx::HitObject' field declared here}}
{}


[shader("geometry")]
[maxvertexcount(12)]
void gs_tri(
    triangle TYPE a,
    // expected-error@-1{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
    inout TriangleStream<TYPE> OutputStream0)
    // expected-error@-1{{object 'dx::HitObject' is not allowed in geometry streams}}
    // expected-note@16{{'dx::HitObject' field declared here}}
{}

[shader("domain")]
[domain("tri")]
void ds_main(
    OutputPatch<TYPE, 3> TrianglePatch)
    // expected-error@-1{{object 'dx::HitObject' is not allowed in tessellation patches}}
    // expected-note@16{{'dx::HitObject' field declared here}}
{}

void patch_const(
    InputPatch<TYPE, 3> inpatch,
    // expected-error@-1{{object 'dx::HitObject' is not allowed in tessellation patches}}
    // expected-note@16{{'dx::HitObject' field declared here}}
    OutputPatch<TYPE, 3> outpatch)
    // expected-error@-1{{object 'dx::HitObject' is not allowed in tessellation patches}}
    // expected-note@16{{'dx::HitObject' field declared here}}
{}

[shader("hull")]
[domain("tri")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(32)]
[patchconstantfunc("patch_const")]
void hs_main(InputPatch<TYPE, 3> TrianglePatch) {}
// expected-error@-1{{object 'dx::HitObject' is not allowed in tessellation patches}}
// expected-note@16{{'dx::HitObject' field declared here}}

RaytracingAccelerationStructure RTAS;

struct [raypayload] DXRHitStruct {
  float4 f : write(closesthit) : read(caller);
  TYPE hit : write(closesthit) : read(caller);
};

struct [raypayload] DXRHitStructSub : DXRHitStruct {
  int3 is : write(closesthit) : read(caller);
};

template<typename T>
struct [raypayload] DXRHitTpl {
  float4 f : write(closesthit) : read(caller);
  T hit : write(closesthit) : read(caller);
};

#define RTTYPE PASTE(DXR,TYPE)


TYPE userFunc(TYPE arg) {
  return arg;
}

[shader("raygeneration")]
void raygen() {
  RTTYPE p = (RTTYPE)0;
  RayDesc ray = (RayDesc)0;
  TraceRay(RTAS, RAY_FLAG_NONE, 0, 0, 1, 0, ray, p); 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in user-defined struct parameter}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  CallShader(0, p);
  // expected-error@-1{{object 'dx::HitObject' is not allowed in user-defined struct parameter}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  TYPE val;
  TYPE res = userFunc(val);
}

[shader("closesthit")]
void closesthit(
    inout RTTYPE payload,
    // expected-error@-1{{payload parameter 'payload' must be a user-defined type composed of only numeric types}}
    // expected-error@-2{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
    in RTTYPE attribs) {
    // expected-error@-1{{attributes parameter 'attribs' must be a user-defined type composed of only numeric types}}
    // expected-error@-2{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
  RayDesc ray;
  TraceRay( RTAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload );
  // expected-error@-1{{object 'dx::HitObject' is not allowed in user-defined struct parameter}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  CallShader(0, payload); 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in user-defined struct parameter}}
  // expected-note@16{{'dx::HitObject' field declared here}}
}

[shader("anyhit")]
void AnyHit(
    inout RTTYPE payload, 
    // expected-error@-1{{payload parameter 'payload' must be a user-defined type composed of only numeric types}}
    // expected-error@-2{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
    in RTTYPE attribs)
    // expected-error@-1{{attributes parameter 'attribs' must be a user-defined type composed of only numeric types}}
    // expected-error@-2{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
{
}

[shader("miss")]
void Miss(
    inout RTTYPE payload){
    // expected-error@-1{{payload parameter 'payload' must be a user-defined type composed of only numeric types}}
    // expected-error@-2{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
  RayDesc ray;
  TraceRay( RTAS, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload ); 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in user-defined struct parameter}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  CallShader(0, payload);
  // expected-error@-1{{object 'dx::HitObject' is not allowed in user-defined struct parameter}}
  // expected-note@16{{'dx::HitObject' field declared here}}
}

[shader("intersection")]
void Intersection() {
  float hitT = RayTCurrent();
  RTTYPE attr = (RTTYPE)0;
  bool bReported = ReportHit(hitT, 0, attr);
  // expected-error@-1{{object 'dx::HitObject' is not allowed in attributes}}
  // expected-note@16{{'dx::HitObject' field declared here}}
}

[shader("callable")]
void callable1(
    inout RTTYPE p) { 
    // expected-error@-1{{object 'dx::HitObject' is not allowed in entry function parameters}}
    // expected-note@16{{'dx::HitObject' field declared here}}
    // expected-error@-3{{callable parameter 'p' must be a user-defined type composed of only numeric types}}
  CallShader(0, p); 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in user-defined struct parameter}}
  // expected-note@16{{'dx::HitObject' field declared here}}
}

static groupshared TYPE gs_var;
// expected-error@-1{{object 'dx::HitObject' is not allowed in groupshared variables}}
// expected-note@16{{'dx::HitObject' field declared here}}

[shader("amplification")]
[numthreads(1,1,1)]
void Amp() {
  TYPE as_pld;
  DispatchMesh(1,1,1,as_pld); 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in user-defined struct parameter}}
  // expected-note@16{{'dx::HitObject' field declared here}}
}

struct NodeHitStruct {
  uint3 grid : SV_DispatchGrid;
  TYPE hit;
};

struct NodeHitStructSub : NodeHitStruct {
  int3 is;
};

template<typename T>
struct NodeHitTpl {
  uint3 grid : SV_DispatchGrid;
  T hit;
};

#define NTYPE PASTE(Node,TYPE)

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeMaxDispatchGrid(8, 1, 1)]
void broadcast(
// expected-error@-1{{Broadcasting node shader 'broadcast' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
    DispatchNodeInputRecord<NTYPE> input,
    // expected-error@-1{{object 'dx::HitObject' is not allowed in node records}}
    // expected-note@16{{'dx::HitObject' field declared here}}
    NodeOutput<TYPE> output)
    // expected-error@-1{{object 'dx::HitObject' is not allowed in node records}}
    // expected-note@16{{'dx::HitObject' field declared here}}
{
  ThreadNodeOutputRecords<TYPE> touts; 
  // expected-error@-1{{object 'dx::HitObject' is not allowed in node records}}
  // expected-note@16{{'dx::HitObject' field declared here}}
  GroupNodeOutputRecords<TYPE> gouts;
  // expected-error@-1{{object 'dx::HitObject' is not allowed in node records}}
  // expected-note@16{{'dx::HitObject' field declared here}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(8,1,1)]
void coalesce(GroupNodeInputRecords<TYPE> input) {}
// expected-error@-1{{object 'dx::HitObject' is not allowed in node records}}
// expected-note@16{{'dx::HitObject' field declared here}}

[Shader("node")]
[NodeLaunch("thread")]
void threader(ThreadNodeInputRecord<TYPE> input) {}
// expected-error@-1{{object 'dx::HitObject' is not allowed in node records}}
// expected-note@16{{'dx::HitObject' field declared here}}
