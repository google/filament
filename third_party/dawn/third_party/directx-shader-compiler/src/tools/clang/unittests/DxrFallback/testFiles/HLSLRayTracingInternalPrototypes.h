#ifndef HLSL_RAYTRACING_INTERNAL_PROTOTYPES
#define HLSL_RAYTRACING_INTERNAL_PROTOTYPES

#define INSTANCE_FLAG_NONE 0x0
#define INSTANCE_FLAG_TRIANGLE_CULL_DISABLE 0x1
#define INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE 0x2
#define INSTANCE_FLAG_FORCE_OPAQUE 0x4
#define INSTANCE_FLAG_FORCE_NON_OPAQUE 0x8

#define SHADER_test [experimental("shader", "test")][noinline]
#define SHADER_internal [experimental("shader", "internal")][noinline]

// Declare ReportHit overload for given attribute structure
#define Declare_Fallback_SetPendingAttr(attr_t)                                \
  void Fallback_SetPendingAttr(attr_t);

// Declare TraceRayTest overload for given attribute structure
#define Declare_TraceRayTest(attr_t)                                           \
  SHADER_test void TraceRayTest(int param, attr_t);

float3 Fallback_WorldRayOrigin();
float3 Fallback_WorldRayDirection();
float Fallback_RayTMin();
float Fallback_RayTCurrent();
uint Fallback_RayFlags();
float3 Fallback_ObjectRayOrigin();
float3 Fallback_ObjectRayDirection();
uint Fallback_PrimitiveIndex();
uint Fallback_InstanceIndex();
uint Fallback_InstanceID();
uint Fallback_HitKind();
uint Fallback_ShaderRecordOffset();

void Fallback_SetWorldRayOrigin(float3 val);
void Fallback_SetWorldRayDirection(float3 val);
void Fallback_SetRayTMin(float val);
void Fallback_SetRayTCurrent(float val);
void Fallback_SetRayFlags(uint rayFlags);
void Fallback_SetObjectRayOrigin(float3 val);
void Fallback_SetObjectRayDirection(float3 val);
void Fallback_SetPrimitiveIndex(uint val);
void Fallback_SetInstanceIndex(uint val);
void Fallback_SetInstanceID(uint val);
void Fallback_SetHitKind(uint val);
void Fallback_SetShaderRecordOffset(uint offset);
void Fallback_SetObjectToWorld(row_major float3x4 val);
void Fallback_SetWorldToObject(row_major float3x4 val);
void Fallback_SetPendingRayTCurrent(float t);
void Fallback_SetPendingHitKind(uint hitKind);
void Fallback_SetPendingTriVals(uint hitGroupRecordOffset, uint primitiveIndex,
                                uint instanceIndex, uint instanceID, float t,
                                uint hitKind);
void Fallback_SetPendingCustomVals(uint hitGroupRecordOffset,
                                   uint primitiveIndex, uint instanceIndex,
                                   uint instanceID);

// Returns the old payload offset to be restored by Fallback_TraceRayEnd().
uint Fallback_TraceRayBegin(uint rayFlags, float3 origin, float tmin,
                            float3 dir, float tmax, uint newPayloadOffset);
void Fallback_TraceRayEnd(int oldPayloadOffset);
uint Fallback_GroupIndex();
int Fallback_AnyHitResult();
void Fallback_SetAnyHitResult(int result);
int Fallback_AnyHitStateId();
void Fallback_SetAnyHitStateId(int stateId);
void Fallback_CommitHit();
void Fallback_CallIndirect(int stateId);
void Fallback_Scheduler(int initialStateId, uint dimx, uint dimy);

int Fallback_RuntimeDataLoadInt(int offset);
void Fallback_RuntimeDataStoreInt(int offset, int val);

#endif // HLSL_RAYTRACING_INTERNAL_PROTOTYPES
