//=================================================================================================================================
// Ray tracing prototype header  (not bothering to separate into a different
// header for now)
//=================================================================================================================================

#ifndef HLSL_RAYTRACING_PROTOTYPES
#define HLSL_RAYTRACING_PROTOTYPES

#define HIT_KIND_TRIANGLE_FRONT_FACE 0xFE
#define HIT_KIND_TRIANGLE_BACK_FACE 0xFF

typedef uint RAY_FLAG;
#define RAY_FLAG_NONE 0x00
#define RAY_FLAG_FORCE_OPAQUE 0x01
#define RAY_FLAG_FORCE_NON_OPAQUE 0x02
#define RAY_FLAG_TERMINATE_ON_FIRST_HIT 0x04
#define RAY_FLAG_SKIP_CLOSEST_HIT_SHADER 0x08
#define RAY_FLAG_CULL_BACK_FACING_TRIANGLES 0x10
#define RAY_FLAG_CULL_FRONT_FACING_TRIANGLES 0x20
#define RAY_FLAG_CULL_OPAQUE 0x40
#define RAY_FLAG_CULL_NON_OPAQUE 0x80

#define SV_RayPayload RT_RayPayload
#define SV_IntersectionAttributes RT_IntersectionAttributes

struct RayDesc {
  float3 Origin;
  float TMin;
  float3 Direction;
  float TMax;
};

struct BuiltInTriangleIntersectionAttributes {
  float2 barycentrics;
};

typedef ByteAddressBuffer RayTracingAccelerationStructure;

// Declare TraceRay overload for given payload structure
//#define Declare_TraceRay(payload_t) \
//    void TraceRay(RayTracingAccelerationStructure, uint RayFlags, uint
//    InstanceCullMask, uint RayContributionToHitGroupIndex, uint
//    MultiplierForGeometryContributionToHitGroupIndex, uint MissShaderIndex,
//    RayDesc, inout payload_t);
#define Declare_TraceRay(payload_t) void TraceRay(int param, inout payload_t);

// Declare ReportHit overload for given attribute structure
#define Declare_ReportHit(attr_t)                                              \
  bool ReportHit(float HitT, uint HitKind, attr_t);

// Declare CallShader overload for given param structure
#define Declare_CallShader(param_t)                                            \
  void CallShader(uint ShaderIndex, inout param_t);

void IgnoreHit();
void AcceptHitAndEndSearch();

// System Value retrieval functions
uint2 DispatchRaysIndex();
uint2 DispatchRaysDimensions();
float3 WorldRayOrigin();
float3 WorldRayDirection();
float RayTMin();
float CurrentRayT();
uint RayFlags();
uint PrimitiveIndex();
uint InstanceIndex();
uint InstanceID();
float3 ObjectRayOrigin();
float3 ObjectRayDirection();
row_major float3x4 ObjectToWorld();
row_major float3x4 WorldToObject();
uint HitKind();

// Place SHADER_* before appropriate entry function
#define SHADER_raygeneration [experimental("shader", "raygeneration")]
#define SHADER_intersection [experimental("shader", "intersection")]
#define SHADER_anyhit [experimental("shader", "anyhit")]
#define SHADER_closesthit [experimental("shader", "closesthit")]
#define SHADER_miss [experimental("shader", "miss")]
#define SHADER_callable [experimental("shader", "callable")]

#endif // HLSL_RAYTRACING_PROTOTYPES
