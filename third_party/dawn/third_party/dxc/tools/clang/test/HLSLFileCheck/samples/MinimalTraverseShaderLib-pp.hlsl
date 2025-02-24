// RUN: %dxc -T lib_6_3 -HV 2017 -O3 -Zpr -default-linkage external %s | FileCheck %s

// CHECK: define void @"\01?Fallback_TraceRay

#line 1 "dxr-fl\\MinimalTraverseShaderLib.hlsl"
#line 13 "dxr-fl\\MinimalTraverseShaderLib.hlsl"
#line 1 "dxr-fl/TraverseShader.hlsli"
#line 11 "dxr-fl/TraverseShader.hlsli"
#line 1 "dxr-fl/HLSLRaytracingInternalPrototypes.h"
#line 24 "dxr-fl/HLSLRaytracingInternalPrototypes.h"
void Fallback_SetWorldRayOrigin(float3 val);
void Fallback_SetWorldRayDirection(float3 val);
void Fallback_SetRayTMin(float val);
void Fallback_SetRayTCurrent(float val);
void Fallback_SetRayFlags(uint rayFlags);
void Fallback_SetPrimitiveIndex(uint val);
void Fallback_SetInstanceIndex(uint val);
void Fallback_SetInstanceID(uint val);
void Fallback_SetObjectRayOrigin(float3 val);
void Fallback_SetObjectRayDirection(float3 val);
void Fallback_SetObjectToWorld(row_major float3x4 val);
void Fallback_SetWorldToObject(row_major float3x4 val);
void Fallback_SetHitKind(uint val);
void Fallback_SetShaderRecordOffset(uint offset);
void Fallback_SetPendingRayTCurrent(float t);
void Fallback_SetPendingHitKind(uint hitKind);
void Fallback_SetPendingTriVals(uint shaderRecordOffset, uint primitiveIndex, uint instanceIndex, uint instanceID, float t, uint hitKind);
void Fallback_SetPendingCustomVals(uint shaderRecordOffset, uint primitiveIndex, uint instanceIndex, uint instanceID);
uint Fallback_SetPayloadOffset(uint payloadOffset);


uint Fallback_TraceRayBegin(uint rayFlags, float3 origin, float tmin, float3 dir, float tmax, uint newPayloadOffset);
void Fallback_TraceRayEnd(int oldPayloadOffset);
uint Fallback_GroupIndex();
uint Fallback_ShaderRecordOffset();
int Fallback_AnyHitResult();
void Fallback_SetAnyHitResult(int result);
int Fallback_AnyHitStateId();
void Fallback_SetAnyHitStateId(int stateId);
void Fallback_CommitHit();
void Fallback_CallIndirect(int stateId);
void Fallback_Scheduler(int initialStateId, uint dimx, uint dimy);



uint Fallback_InstanceIndex();
float Fallback_RayTCurrent();
#line 11 "dxr-fl/TraverseShader.hlsli"




#line 1 "dxr-fl/UberShaderBindings.h"
#line 12 "dxr-fl/UberShaderBindings.h"
#line 1 "dxr-fl/FallbackDebug.h"
#line 12 "dxr-fl/UberShaderBindings.h"

#line 1 "dxr-fl/FallbackDxil.h"
#line 13 "dxr-fl/UberShaderBindings.h"





#line 1 "dxr-fl/ShaderUtil.hlsli"
#line 18 "dxr-fl/UberShaderBindings.h"






struct DebugVariables
{
    uint LevelToVisualize;
};



cbuffer Constants : register(b0, space214743647)
{
    uint RayDispatchDimensionsWidth;
    uint RayDispatchDimensionsHeight;
    uint HitGroupShaderRecordStride;
    uint MissShaderRecordStride;

    uint SamplerDescriptorHeapStartLo;
    uint SamplerDescriptorHeapStartHi;
    uint SrvCbvUavDescriptorHeapStartLo;
    uint SrvCbvUavDescriptorHeapStartHi;
};

cbuffer AccelerationStructureList : register(b1, space214743647)
{
    uint2 TopLevelAccelerationStructureGpuVA;
};

ByteAddressBuffer HitGroupShaderTable : register(t0, space214743647);
ByteAddressBuffer MissShaderTable : register(t1, space214743647);
ByteAddressBuffer RayGenShaderTable : register(t2, space214743647);
ByteAddressBuffer CallableShaderTable : register(t3, space214743647);

RWByteAddressBuffer DescriptorHeapBufferTable[] : register(u0, space214743648);
#line 15 "dxr-fl/TraverseShader.hlsli"

#line 1 "dxr-fl/DebugLog.h"
#line 27 "dxr-fl/DebugLog.h"
void BeginLog();
void LogInt(int val);
void LogInt2(int2 val);
void LogInt3(int3 val);
void LogFloat(float val);
void LogFloat3(float3 val);
void LogTraceRayStart();
void LogTraceRayEnd();
#line 16 "dxr-fl/TraverseShader.hlsli"

#line 1 "dxr-fl/RayTracingHelper.hlsli"
#line 14 "dxr-fl/RayTracingHelper.hlsli"
#line 1 "dxr-fl/EmulatedPointer.hlsli"
#line 18 "dxr-fl/EmulatedPointer.hlsli"
uint2 PointerAdd(uint2 address, uint offset)
{
    address[0] += offset;
    return address;
}

struct RWByteAddressBufferPointer
{
    RWByteAddressBuffer buffer;
    uint offsetInBytes;
};

static
RWByteAddressBufferPointer CreateRWByteAddressBufferPointer(in RWByteAddressBuffer buffer, uint offsetInBytes)
{
    RWByteAddressBufferPointer pointer;
    pointer.buffer = buffer;
    pointer.offsetInBytes = offsetInBytes;
    return pointer;
}
#line 14 "dxr-fl/RayTracingHelper.hlsli"

#line 1 "dxr-fl/RayTracingHlslCompat.h"
#line 13 "dxr-fl/RayTracingHlslCompat.h"
#line 1 "dxr-fl/WaveDimensions.h"
#line 13 "dxr-fl/RayTracingHlslCompat.h"
#line 46 "dxr-fl/RayTracingHlslCompat.h"
struct HierarchyNode
{

    uint ParentIndex;




    uint LeftChildIndex;
    uint RightChildIndex;

    static const int IsCollapseChildren = 0x80000000;
};

struct AABB
{

    float3 min;
    float3 max;
#line 77 "dxr-fl/RayTracingHlslCompat.h"
};


void AABBToRawData(in AABB aabb, out uint4 a, out uint2 b)
{
    a = asuint(float4(aabb.min.xyz, aabb.max.x));
    b = asuint(float2(aabb.max.yz));
}




struct Triangle
{

    float3 v0;
    float3 v1;
    float3 v2;
#line 107 "dxr-fl/RayTracingHlslCompat.h"
};



Triangle RawDataToTriangle(uint4 a, uint4 b, uint c)
{
    Triangle tri;
    tri.v0 = asfloat(a.xyz);
    tri.v1 = asfloat(uint3(a.w, b.xy));
    tri.v2 = asfloat(uint3(b.zw, c));

    return tri;
}

void TriangleToRawData(in Triangle tri, out uint4 a, out uint4 b, out uint c)
{
    a = asuint(float4(tri.v0.xyz, tri.v1.x));
    b = asuint(float4(tri.v1.yz, tri.v2.xy));
    c = asuint(tri.v2.z);
}






struct Primitive
{
    uint PrimitiveType;

    uint4 data0;
    uint4 data1;
    uint data2;







};

Primitive NullPrimitive()
{
    Primitive primitive;
    primitive.PrimitiveType = 0;
    primitive.data0 = 0;
    primitive.data1 = 0;
    primitive.data2 = 0;
    return primitive;
}

Primitive CreateProceduralGeometryPrimitive(AABB aabb)
{
    Primitive primitive = NullPrimitive();
    primitive.PrimitiveType = 0x2;
    AABBToRawData(aabb, primitive.data0, primitive.data1.xy);
    return primitive;
}

Primitive CreateTrianglePrimitive(Triangle tri)
{
    Primitive primitive = NullPrimitive();
    primitive.PrimitiveType = 0x1;
    TriangleToRawData(tri, primitive.data0, primitive.data1, primitive.data2);
    return primitive;
}

Triangle GetTriangle(Primitive prim)
{
    return RawDataToTriangle(prim.data0, prim.data1, prim.data2);
}

AABB GetProceduralPrimitiveAABB(Primitive prim)
{
    AABB aabb;
    aabb.min = asfloat(prim.data0.xyz);
    aabb.max = asfloat(uint3(prim.data0.w, prim.data1.xy));

    return aabb;
}
#line 197 "dxr-fl/RayTracingHlslCompat.h"
struct PrimitiveMetaData
{
    uint GeometryContributionToHitGroupIndex;
    uint PrimitiveIndex;
    uint GeometryFlags;
};
#line 213 "dxr-fl/RayTracingHlslCompat.h"
float3x4 CreateMatrix(float4 rows[3])
{
    float3x4 mat;
    mat[0] = rows[0];
    mat[1] = rows[1];
    mat[2] = rows[2];
    return mat;
}

static const uint D3D12_RAYTRACING_INSTANCE_FLAG_NONE = 0;
static const uint D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 0x1;
static const uint D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x2;
static const uint D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE = 0x4;
static const uint D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE = 0x8;

static const uint D3D12_RAYTRACING_GEOMETRY_FLAG_NONE = 0;
static const uint D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE = 0x1;
static const uint D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION = 0x2;

struct RaytracingInstanceDesc
{
    float4 Transform[3];
    uint InstanceIDAndMask;
    uint InstanceContributionToHitGroupIndexAndFlags;
    uint2 AccelerationStructure;
};


struct BVHMetadata
{

    RaytracingInstanceDesc instanceDesc;



    float4 ObjectToWorld[3];
    uint InstanceIndex;
};



static
void StoreBVHMetadataToRawData(RWByteAddressBuffer buffer, uint offset, BVHMetadata metadata)
{
    uint4 data[7];
    uint dataRemainder;

    data[0] = asuint(metadata.instanceDesc.Transform[0]);
    data[1] = asuint(metadata.instanceDesc.Transform[1]);
    data[2] = asuint(metadata.instanceDesc.Transform[2]);
    data[3].x = metadata.instanceDesc.InstanceIDAndMask;
    data[3].y = metadata.instanceDesc.InstanceContributionToHitGroupIndexAndFlags;
    data[3].zw = metadata.instanceDesc.AccelerationStructure;
    data[4] = asuint(metadata.ObjectToWorld[0]);
    data[5] = asuint(metadata.ObjectToWorld[1]);
    data[6] = asuint(metadata.ObjectToWorld[2]);
    dataRemainder = metadata.InstanceIndex;

    [unroll]
    for (uint i = 0; i < 7; i++)
    {
        buffer.Store4(offset, data[i]);
        offset += 16;
    }
    buffer.Store(offset, dataRemainder);
}

RaytracingInstanceDesc RawDataToRaytracingInstanceDesc(uint4 a, uint4 b, uint4 c, uint4 d)
{
    RaytracingInstanceDesc desc;
    desc.Transform[0] = asfloat(a);
    desc.Transform[1] = asfloat(b);
    desc.Transform[2] = asfloat(c);

    desc.InstanceIDAndMask = d.x;
    desc.InstanceContributionToHitGroupIndexAndFlags = d.y;
    desc.AccelerationStructure = d.zw;

    return desc;
}

static
BVHMetadata LoadBVHMetadata(RWByteAddressBuffer buffer, uint offset)
{
    uint4 data[7];
    [unroll]
    for (uint i = 0; i < 7; i++)
    {
        data[i] = buffer.Load4(offset);
        offset += 16;
    }
    BVHMetadata metadata;
    metadata.instanceDesc = RawDataToRaytracingInstanceDesc(data[0], data[1], data[2], data[3]);
    metadata.ObjectToWorld[0] = asfloat(data[4]);
    metadata.ObjectToWorld[1] = asfloat(data[5]);
    metadata.ObjectToWorld[2] = asfloat(data[6]);
    metadata.InstanceIndex = buffer.Load(offset);

    return metadata;
}

static
RaytracingInstanceDesc LoadRaytracingInstanceDesc(RWByteAddressBuffer buffer, uint offset)
{
    uint4 data[4];
    [unroll]
    for (uint i = 0; i < 4; i++)
    {
        data[i] = buffer.Load4(offset + 16 * i);
    }
    return RawDataToRaytracingInstanceDesc(data[0], data[1], data[2], data[3]);
}

static
RaytracingInstanceDesc LoadRaytracingInstanceDesc(ByteAddressBuffer buffer, uint offset)
{
    uint4 data[4];
    [unroll]
    for (uint i = 0; i < 4; i++)
    {
        data[i] = buffer.Load4(offset + 16 * i);
    }
    return RawDataToRaytracingInstanceDesc(data[0], data[1], data[2], data[3]);
}

uint GetInstanceContributionToHitGroupIndex(RaytracingInstanceDesc desc)
{
    return desc.InstanceContributionToHitGroupIndexAndFlags & 0xffffff;
}

uint GetInstanceFlags(RaytracingInstanceDesc desc)
{
    return desc.InstanceContributionToHitGroupIndexAndFlags >> 24;
}

uint GetInstanceMask(RaytracingInstanceDesc desc)
{
    return desc.InstanceIDAndMask >> 24;
}

uint GetInstanceID(RaytracingInstanceDesc desc)
{
  return desc.InstanceIDAndMask & 0xFFFFFF;
}






struct AABBNodeSibling
{

    uint childIndex;
#line 375 "dxr-fl/RayTracingHlslCompat.h"
    float center[3];
    float halfDim[3];


    uint primitiveFlags;
#line 389 "dxr-fl/RayTracingHlslCompat.h"
};





struct AABBNode
{
    AABBNodeSibling left;
    AABBNodeSibling right;
};






struct BVHOffsets
{
    uint offsetToBoxes;
    uint offsetToVertices;
    uint offsetToPrimitiveMetaData;
    uint totalSize;
};





inline
uint GetNumInternalNodes(uint numLeaves)
{
    return numLeaves - 1;
}

inline
uint GetNumAABBNodes(uint numLeaves)
{
    return numLeaves <= 1 ? 1 : GetNumInternalNodes(numLeaves);
}

inline
uint GetOffsetFromSortedIndicesToAABBParents(uint numPrimitives) {
    return 4 * numPrimitives;
}

inline
uint GetOffsetToBVHMetadata(uint numElements)
{
    return (4 * 4) + GetNumAABBNodes(numElements) * ((8 * 4) * 2);
}

inline
uint GetOffsetToBVHSortedIndices(uint numElements) {
    return GetOffsetToBVHMetadata(numElements) + 116 * numElements;
}

inline
uint GetOffsetFromPrimitiveMetaDataToSortedIndices(uint numPrimitives)
{
    return (4 * 3) * numPrimitives;
}

inline
uint GetOffsetFromPrimitivesToPrimitiveMetaData(uint numPrimitives)
{
    return 40 * numPrimitives;
}

inline
uint GetOffsetToPrimitives(uint numElements)
{
    return (4 * 4) + GetNumAABBNodes(numElements) * ((8 * 4) * 2);
}
#line 15 "dxr-fl/RayTracingHelper.hlsli"

#line 1 "dxr-fl/ShaderUtil.hlsli"
#line 16 "dxr-fl/RayTracingHelper.hlsli"







static const int IsLeafFlag = 0x80000000;
static const int IsProceduralGeometryFlag = 0x40000000;
static const int IsDummyFlag = 0x20000000;
static const int LeafFlags = IsLeafFlag | IsProceduralGeometryFlag | IsDummyFlag;
static const int MinNumberOfPrimitives = 1;
static const int MinNumberOfLeafNodeBVHs = 1;
#line 43 "dxr-fl/RayTracingHelper.hlsli"
static const int OffsetToBoxesOffset = 0;


static const int OffsetToPrimitivesOffset = 4;
static const int OffsetToPrimitiveMetaDataOffset = 8;


static const int OffsetToLeafNodeMetaDataOffset = 4;

static const int OffsetToTotalSize = 12;

int GetLeafIndexFromFlag(uint2 flag)
{
    return flag.x & ~LeafFlags;
}

uint GetActualParentIndex(uint index)
{


    return index & ~HierarchyNode::IsCollapseChildren;



}


struct BoundingBox
{
    float3 center;
    float3 halfDim;
};







static
int GetOffsetToAABBNodes(RWByteAddressBufferPointer pointer)
{



    return pointer.offsetInBytes + (4 * 4);
}

static
int GetOffsetToVertices(RWByteAddressBufferPointer pointer)
{
    return pointer.buffer.Load(OffsetToPrimitivesOffset + pointer.offsetInBytes) + pointer.offsetInBytes;
}

static
int GetOffsetToPrimitiveMetaData(RWByteAddressBufferPointer pointer)
{
    return pointer.buffer.Load(OffsetToPrimitiveMetaDataOffset + pointer.offsetInBytes) + pointer.offsetInBytes;
}

bool IsLeaf(uint2 info)
{
    return (info.y & IsLeafFlag);
}

bool IsProceduralGeometry(uint2 info)
{
    return (info.y & IsProceduralGeometryFlag);
}

bool IsDummy(uint2 info)
{
    return (info.y & IsDummyFlag);
}

uint GetLeafIndexFromInfo(uint2 info)
{
    return info.x;
}

uint GetChildIndexFromInfo(uint2 info)
{
    return info.x;
}

uint GetLeafFlagsFromPrimitiveFlags(uint flags)
{
    return flags & LeafFlags;
}

uint GetNumPrimitivesFromPrimitiveFlags(uint flags)
{
    return flags & ~LeafFlags;
}

uint GetNumPrimitivesFromInfo(uint2 info)
{
    return GetNumPrimitivesFromPrimitiveFlags(info.y);
}

uint CombinePrimitiveFlags(uint flags1, uint flags2)
{
    uint combinedFlags = GetLeafFlagsFromPrimitiveFlags(flags1)
                           | GetLeafFlagsFromPrimitiveFlags(flags2);
    combinedFlags &= ~(IsLeafFlag | IsDummyFlag);

    uint combinedPrimitives = GetNumPrimitivesFromPrimitiveFlags(flags1)
                            + GetNumPrimitivesFromPrimitiveFlags(flags2);

    return combinedFlags | combinedPrimitives;
}

uint GetAABBNodeAddress(uint startAddress, uint boxIndex)
{
    return startAddress + boxIndex * ((8 * 4) * 2);
}

BoundingBox CreateDummyBox(out uint2 info)
{
    BoundingBox box;
    info = uint2(0, IsLeafFlag | IsDummyFlag);
    return box;
}

static
void CompressBox(BoundingBox box, uint childIndex, uint primitiveFlags, out uint4 data1, out uint4 data2)
{
    data1.x = childIndex;
    data1.y = asuint(box.center.x);
    data1.z = asuint(box.center.y);
    data1.w = asuint(box.center.z);

    data2.x = asuint(box.halfDim.x);
    data2.y = asuint(box.halfDim.y);
    data2.z = asuint(box.halfDim.z);
    data2.w = primitiveFlags;
}

static
void WriteBoxToBuffer(
    RWByteAddressBuffer buffer,
    uint boxAddress,
    BoundingBox box,
    uint2 extraInfo)
{
    uint4 data1, data2;
    CompressBox(box, extraInfo.x, extraInfo.y, data1, data2);

    buffer.Store4(boxAddress, data1);
    buffer.Store4(boxAddress + 16, data2);
}

static
void WriteLeftBoxToBuffer(
    RWByteAddressBuffer buffer,
    uint nodeStartOffset,
    uint nodeIndex,
    BoundingBox box,
    uint2 extraInfo)
{
    uint boxAddress = GetAABBNodeAddress(nodeStartOffset, nodeIndex);
    WriteBoxToBuffer(buffer, boxAddress, box, extraInfo);
}

static
void WriteRightBoxToBuffer(
    RWByteAddressBuffer buffer,
    uint nodeStartOffset,
    uint nodeIndex,
    BoundingBox box,
    uint2 extraInfo)
{
    uint boxAddress = GetAABBNodeAddress(nodeStartOffset, nodeIndex) + (8 * 4);
    WriteBoxToBuffer(buffer, boxAddress, box, extraInfo);
}

static
BoundingBox RawDataToBoundingBox(uint4 a, uint4 b, out uint2 extraInfo)
{
    BoundingBox box;
    box.center.x = asfloat(a.y);
    box.center.y = asfloat(a.z);
    box.center.z = asfloat(a.w);
    box.halfDim.x = asfloat(b.x);
    box.halfDim.y = asfloat(b.y);
    box.halfDim.z = asfloat(b.z);
    extraInfo.x = a.x;
    extraInfo.y = b.w;
    return box;
}

static
BoundingBox GetBoxFromBuffer(RWByteAddressBuffer buffer, uint aabbNodeAddress, out uint2 extraInfo)
{
    uint4 data1 = buffer.Load4(aabbNodeAddress);
    uint4 data2 = buffer.Load4(aabbNodeAddress + 16);

    return RawDataToBoundingBox(data1, data2, extraInfo);
}

static
BoundingBox GetLeftBoxFromBuffer(
    RWByteAddressBuffer buffer,
    uint nodeStartOffset,
    uint parentNodeIndex,
    out uint2 extraInfo)
{
    uint aabbNodeAddress = GetAABBNodeAddress(nodeStartOffset, parentNodeIndex);
    return GetBoxFromBuffer(buffer, aabbNodeAddress, extraInfo);
}

static
BoundingBox GetRightBoxFromBuffer(
    RWByteAddressBuffer buffer,
    uint nodeStartOffset,
    uint parentNodeIndex,
    out uint2 extraInfo)
{
    uint aabbNodeAddress = GetAABBNodeAddress(nodeStartOffset, parentNodeIndex) + (8 * 4);
    return GetBoxFromBuffer(buffer, aabbNodeAddress, extraInfo);
}

static
BoundingBox GetLeftBoxFromBVH(RWByteAddressBufferPointer pointer, int nodeIndex, out uint2 extraInfo)
{
    uint nodeStartOffset = GetOffsetToAABBNodes(pointer);
    uint aabbNodeAddress = GetAABBNodeAddress(nodeStartOffset, nodeIndex);
    return GetBoxFromBuffer(pointer.buffer, aabbNodeAddress, extraInfo);
}

static
BoundingBox GetRightBoxFromBVH(RWByteAddressBufferPointer pointer, int nodeIndex, out uint2 extraInfo)
{
    uint nodeStartOffset = GetOffsetToAABBNodes(pointer);
    uint aabbNodeAddress = GetAABBNodeAddress(nodeStartOffset, nodeIndex) + (8 * 4);
    return GetBoxFromBuffer(pointer.buffer, aabbNodeAddress, extraInfo);
}
#line 288 "dxr-fl/RayTracingHelper.hlsli"
uint GetPrimitiveMetaDataAddress(uint startAddress, uint triangleIndex)
{
    return startAddress + triangleIndex * (4 * 3);
}

static
PrimitiveMetaData BVHReadPrimitiveMetaData(RWByteAddressBufferPointer pointer, int primitiveIndex)
{
    const uint readAddress = GetPrimitiveMetaDataAddress(GetOffsetToPrimitiveMetaData(pointer), primitiveIndex);

    PrimitiveMetaData metadata;
    const uint3 a = pointer.buffer.Load3(readAddress);
    metadata.GeometryContributionToHitGroupIndex = a.x;
    metadata.PrimitiveIndex = a.y;
    metadata.GeometryFlags = a.z;
    return metadata;
}

static
void BVHReadTriangle(
    RWByteAddressBufferPointer pointer,
    out float3 v0,
    out float3 v1,
    out float3 v2,
    uint triId)
{
    uint baseOffset = GetOffsetToVertices(pointer) + triId * 40
        + 4;

    const float4 a = asfloat(pointer.buffer.Load4(baseOffset));
    const float4 b = asfloat(pointer.buffer.Load4(baseOffset + 16));
    const float c = asfloat(pointer.buffer.Load(baseOffset + 32));

    v0 = a.xyz;
    v1 = float3(a.w, b.xy);
    v2 = float3(b.zw, c);
}

static
BoundingBox AABBtoBoundingBox(AABB aabb)
{
    BoundingBox box;
    box.center = (aabb.min + aabb.max) * 0.5f;
    box.halfDim = aabb.max - box.center;
    return box;
}

static
AABB BoundingBoxToAABB(BoundingBox boundingBox)
{
    AABB aabb;
    aabb.min = boundingBox.center - boundingBox.halfDim;
    aabb.max = boundingBox.center + boundingBox.halfDim;
    return aabb;
}

static
AABB RawDataToAABB(int4 a, int4 b)
{
    uint2 unusedInfo;
    return BoundingBoxToAABB(RawDataToBoundingBox(a, b, unusedInfo));
}

static
BoundingBox GetBoxDataFromTriangle(float3 v0, float3 v1, float3 v2, int triangleIndex, out uint2 triangleInfo)
{
    AABB aabb;
    aabb.min = min(min(v0, v1), v2);
    aabb.max = max(max(v0, v1), v2);

    aabb.min = min(aabb.min, aabb.max - 0.001);

    BoundingBox box = AABBtoBoundingBox(aabb);
    triangleInfo.x = triangleIndex;
    triangleInfo.y = IsLeafFlag | MinNumberOfPrimitives;
    return box;
}

float3 GetMinCorner(BoundingBox box)
{
    return box.center - box.halfDim;
}

float3 GetMaxCorner(BoundingBox box)
{
    return box.center + box.halfDim;
}

AABB GetAABBFromChildBoxes(BoundingBox boxA, BoundingBox boxB)
{
    AABB aabb;
    aabb.min = min(GetMinCorner(boxA), GetMinCorner(boxB));
    aabb.max = max(GetMaxCorner(boxA), GetMaxCorner(boxB));

    return aabb;
}

BoundingBox GetBoxFromChildBoxes(BoundingBox boxA, BoundingBox boxB)
{
    return AABBtoBoundingBox(GetAABBFromChildBoxes(boxA, boxB));
}

float Determinant(in float3x4 transform)
{
    return transform[0][0] * transform[1][1] * transform[2][2] -
        transform[0][0] * transform[2][1] * transform[1][2] -
        transform[1][0] * transform[0][1] * transform[2][2] +
        transform[1][0] * transform[2][1] * transform[0][2] +
        transform[2][0] * transform[0][1] * transform[1][2] -
        transform[2][0] * transform[1][1] * transform[0][2];
}

float3x4 InverseAffineTransform(float3x4 transform)
{
    const float invDet = rcp(Determinant(transform));

    float3x4 invertedTransform;
    invertedTransform[0][0] = invDet * (transform[1][1] * (transform[2][2] * 1.0f - 0.0f * transform[2][3]) + transform[2][1] * (0.0f * transform[1][3] - transform[1][2] * 1.0f) + 0.0f * (transform[1][2] * transform[2][3] - transform[2][2] * transform[1][3]));
    invertedTransform[1][0] = invDet * (transform[1][2] * (transform[2][0] * 1.0f - 0.0f * transform[2][3]) + transform[2][2] * (0.0f * transform[1][3] - transform[1][0] * 1.0f) + 0.0f * (transform[1][0] * transform[2][3] - transform[2][0] * transform[1][3]));
    invertedTransform[2][0] = invDet * (transform[1][3] * (transform[2][0] * 0.0f - 0.0f * transform[2][1]) + transform[2][3] * (0.0f * transform[1][1] - transform[1][0] * 0.0f) + 1.0f * (transform[1][0] * transform[2][1] - transform[2][0] * transform[1][1]));
    invertedTransform[0][1] = invDet * (transform[2][1] * (transform[0][2] * 1.0f - 0.0f * transform[0][3]) + 0.0f * (transform[2][2] * transform[0][3] - transform[0][2] * transform[2][3]) + transform[0][1] * (0.0f * transform[2][3] - transform[2][2] * 1.0f));
    invertedTransform[1][1] = invDet * (transform[2][2] * (transform[0][0] * 1.0f - 0.0f * transform[0][3]) + 0.0f * (transform[2][0] * transform[0][3] - transform[0][0] * transform[2][3]) + transform[0][2] * (0.0f * transform[2][3] - transform[2][0] * 1.0f));
    invertedTransform[2][1] = invDet * (transform[2][3] * (transform[0][0] * 0.0f - 0.0f * transform[0][1]) + 1.0f * (transform[2][0] * transform[0][1] - transform[0][0] * transform[2][1]) + transform[0][3] * (0.0f * transform[2][1] - transform[2][0] * 0.0f));
    invertedTransform[0][2] = invDet * (0.0f * (transform[0][2] * transform[1][3] - transform[1][2] * transform[0][3]) + transform[0][1] * (transform[1][2] * 1.0f - 0.0f * transform[1][3]) + transform[1][1] * (0.0f * transform[0][3] - transform[0][2] * 1.0f));
    invertedTransform[1][2] = invDet * (0.0f * (transform[0][0] * transform[1][3] - transform[1][0] * transform[0][3]) + transform[0][2] * (transform[1][0] * 1.0f - 0.0f * transform[1][3]) + transform[1][2] * (0.0f * transform[0][3] - transform[0][0] * 1.0f));
    invertedTransform[2][2] = invDet * (1.0f * (transform[0][0] * transform[1][1] - transform[1][0] * transform[0][1]) + transform[0][3] * (transform[1][0] * 0.0f - 0.0f * transform[1][1]) + transform[1][3] * (0.0f * transform[0][1] - transform[0][0] * 0.0f));
    invertedTransform[0][3] = invDet * (transform[0][1] * (transform[2][2] * transform[1][3] - transform[1][2] * transform[2][3]) + transform[1][1] * (transform[0][2] * transform[2][3] - transform[2][2] * transform[0][3]) + transform[2][1] * (transform[1][2] * transform[0][3] - transform[0][2] * transform[1][3]));
    invertedTransform[1][3] = invDet * (transform[0][2] * (transform[2][0] * transform[1][3] - transform[1][0] * transform[2][3]) + transform[1][2] * (transform[0][0] * transform[2][3] - transform[2][0] * transform[0][3]) + transform[2][2] * (transform[1][0] * transform[0][3] - transform[0][0] * transform[1][3]));
    invertedTransform[2][3] = invDet * (transform[0][3] * (transform[2][0] * transform[1][1] - transform[1][0] * transform[2][1]) + transform[1][3] * (transform[0][0] * transform[2][1] - transform[2][0] * transform[0][1]) + transform[2][3] * (transform[1][0] * transform[0][1] - transform[0][0] * transform[1][1]));

    return invertedTransform;
}

AABB TransformAABB(AABB box, float3x4 transform)
{



    const uint verticesPerAABB = 8;
    float4 boxVertices[verticesPerAABB];
    boxVertices[0] = float4(box.min, 1.0);
    boxVertices[1] = float4(box.min.xy, box.max.z, 1.0);
    boxVertices[2] = float4(box.min.x, box.max.yz, 1.0);
    boxVertices[3] = float4(box.min.x, box.max.y, box.min.z, 1.0);
    boxVertices[4] = float4(box.max.x, box.min.yz, 1.0);
    boxVertices[6] = float4(box.max.x, box.min.y, box.max.z, 1.0);
    boxVertices[5] = float4(box.max.xy, box.min.z, 1.0);
    boxVertices[7] = float4(box.max, 1.0);

    AABB transformedBox;
    transformedBox.min = float3(asfloat(0x7F7FFFFF), asfloat(0x7F7FFFFF), asfloat(0x7F7FFFFF));
    transformedBox.max = float3(-asfloat(0x7F7FFFFF), -asfloat(0x7F7FFFFF), -asfloat(0x7F7FFFFF));
    for (uint i = 0; i < verticesPerAABB; i++)
    {
        float3 tranformedVertex = mul(transform, boxVertices[i]);
        transformedBox.min = min(transformedBox.min, tranformedVertex);
        transformedBox.max = max(transformedBox.max, tranformedVertex);
    }
    return transformedBox;
}

static const uint OffsetToAnyHitStateId = 4;
static const uint OffsetToIntersectionStateId = 8;

static
uint GetAnyHitStateId(ByteAddressBuffer shaderTable, uint recordOffset)
{
    return shaderTable.Load(recordOffset + OffsetToAnyHitStateId);
}

static
void GetAnyHitAndIntersectionStateId(ByteAddressBuffer shaderTable, uint recordOffset, out uint AnyHitStateId, out uint IntersectionStateId)
{
    uint2 stateIds = shaderTable.Load2(recordOffset + OffsetToAnyHitStateId);
    AnyHitStateId = stateIds.x;
    IntersectionStateId = stateIds.y;
}
#line 17 "dxr-fl/TraverseShader.hlsli"

#line 1 "dxr-fl/EmulatedPointerIntrinsics.hlsli"
#line 14 "dxr-fl/EmulatedPointerIntrinsics.hlsli"
static
RWByteAddressBuffer PointerGetBuffer(uint2 address)
{
    return DescriptorHeapBufferTable[NonUniformResourceIndex(address[1])];
}

uint PointerGetBufferStartOffset(uint2 address)
{
    return address[0];
}

uint4 Load4(uint2 address)
{
    return PointerGetBuffer(address).Load4(PointerGetBufferStartOffset(address));
}

static
RWByteAddressBufferPointer CreateRWByteAddressBufferPointerFromGpuVA(uint2 address)
{
    return CreateRWByteAddressBufferPointer(PointerGetBuffer(address), PointerGetBufferStartOffset(address));
}
#line 18 "dxr-fl/TraverseShader.hlsli"

#line 1 "dxr-fl/TraverseFunction.hlsli"
#line 19 "dxr-fl/TraverseFunction.hlsli"
static
uint2 stacks[2][32];
#line 48 "dxr-fl/TraverseFunction.hlsli"
void RecordClosestBox(uint currentLevel, inout bool leftTest, float leftT, inout bool rightTest, float rightT, inout float closestBoxT)
{
#line 66 "dxr-fl/TraverseFunction.hlsli"
}

void StackPush(inout int stackTop, uint level, uint2 nodeInfo, uint depth)
{
    uint stackIndex = stackTop++;





    stacks[level][stackIndex] = nodeInfo;
}

uint2 StackPop(inout int stackTop, uint level, out uint depth)
{
    uint stackIndex = --stackTop;





    return stacks[level][stackIndex];
}

int InvokeAnyHit(int stateId)
{
    Fallback_SetAnyHitResult(1);
    Fallback_CallIndirect(stateId);
    return Fallback_AnyHitResult();
}

void Fallback_IgnoreHit()
{
    Fallback_SetAnyHitResult(0);
}

void Fallback_AcceptHitAndEndSearch()
{
    Fallback_SetAnyHitResult(-1);
}

bool IsOpaque(bool geomOpaque, uint instanceFlags, uint rayFlags)
{
    bool opaque = geomOpaque;

    if (instanceFlags & 0x4)
        opaque = true;
    else if (instanceFlags & 0x8)
        opaque = false;

    if (rayFlags & RAY_FLAG_FORCE_OPAQUE)
        opaque = true;
    else if (rayFlags & RAY_FLAG_FORCE_NON_OPAQUE)
        opaque = false;

    return opaque;
}

int Fallback_ReportHit(float tHit, uint hitKind)
{
    if (tHit < RayTMin() || Fallback_RayTCurrent() <= tHit)
        return 0;

    Fallback_SetPendingRayTCurrent(tHit);
    Fallback_SetPendingHitKind(hitKind);
    int stateId = Fallback_AnyHitStateId();
    int ret = 1;

    bool geomOpaque = true;

    if (stateId > 0 && !IsOpaque(geomOpaque, 0, RayFlags()))
        ret = InvokeAnyHit(stateId);

    if (ret != 0)
    {
        Fallback_CommitHit();
        if (RayFlags() & RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH)
            ret = -1;
    }
    return ret;
}


void UpdateObjectSpaceProperties(float3 objectRayOrigin, float3 objectRayDirection, float3x4 worldToObject, float3x4 objectToWorld )
{
    Fallback_SetObjectRayOrigin(objectRayOrigin);
    Fallback_SetObjectRayDirection(objectRayDirection);
    Fallback_SetWorldToObject(worldToObject);
    Fallback_SetObjectToWorld(objectToWorld);
}





inline
bool RayBoxTest(
    out float resultT,
    float closestT,
    float3 rayOriginTimesRayInverseDirection,
    float3 rayInverseDirection,
    float3 boxCenter,
    float3 boxHalfDim)
{
    const float3 relativeMiddle = boxCenter * rayInverseDirection - rayOriginTimesRayInverseDirection;
    const float3 maxL = relativeMiddle + boxHalfDim * abs(rayInverseDirection);
    const float3 minL = relativeMiddle - boxHalfDim * abs(rayInverseDirection);

    const float minT = max(max(minL.x, minL.y), minL.z);
    const float maxT = min(min(maxL.x, maxL.y), maxL.z);

    resultT = max(minT, 0);
    return max(minT, 0) < min(maxT, closestT);
}

float3 Swizzle(float3 v, int3 swizzleOrder)
{
    return float3(v[swizzleOrder.x], v[swizzleOrder.y], v[swizzleOrder.z]);
}

bool IsPositive(float f) { return f > 0.0f; }


inline
void RayTriangleIntersect(
    inout float hitT,
    in uint instanceFlags,
    out float2 bary,
    float3 rayOrigin,
    float3 rayDirection,
    int3 swizzledIndicies,
    float3 shear,
    float3 v0,
    float3 v1,
    float3 v2)
{

    bool useCulling = !(instanceFlags & D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE);
    bool flipFaces = instanceFlags & D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE;
    uint backFaceCullingFlag = flipFaces ? RAY_FLAG_CULL_FRONT_FACING_TRIANGLES : RAY_FLAG_CULL_BACK_FACING_TRIANGLES;
    uint frontFaceCullingFlag = flipFaces ? RAY_FLAG_CULL_BACK_FACING_TRIANGLES : RAY_FLAG_CULL_FRONT_FACING_TRIANGLES;
    bool useBackfaceCulling = useCulling && (RayFlags() & backFaceCullingFlag);
    bool useFrontfaceCulling = useCulling && (RayFlags() & frontFaceCullingFlag);

    float3 A = Swizzle(v0 - rayOrigin, swizzledIndicies);
    float3 B = Swizzle(v1 - rayOrigin, swizzledIndicies);
    float3 C = Swizzle(v2 - rayOrigin, swizzledIndicies);

    A.xy = A.xy - shear.xy * A.z;
    B.xy = B.xy - shear.xy * B.z;
    C.xy = C.xy - shear.xy * C.z;
    precise float U = C.x * B.y - C.y * B.x;
    precise float V = A.x * C.y - A.y * C.x;
    precise float W = B.x * A.y - B.y * A.x;

    float det = U + V + W;
    if (useFrontfaceCulling)
    {
        if (U > 0.0f || V > 0.0f || W > 0.0f) return;
    }
    else if (useBackfaceCulling)
    {
        if (U < 0.0f || V < 0.0f || W < 0.0f) return;
    }
    else
    {
        if ((U < 0.0f || V < 0.0f || W < 0.0f) &&
            (U > 0.0f || V > 0.0f || W > 0.0f)) return;
    }

    if (det == 0.0f) return;
    A.z = shear.z * A.z;
    B.z = shear.z * B.z;
    C.z = shear.z * C.z;
    const float T = U * A.z + V * B.z + W * C.z;

    if (useFrontfaceCulling)
    {
        if (T > 0.0f || T < hitT * det)
            return;
    }
    else if (useBackfaceCulling)
    {
        if (T < 0.0f || T > hitT * det)
            return;
    }
    else
    {
        float signCorrectedT = abs(T);
        if (IsPositive(T) != IsPositive(det))
        {
            signCorrectedT = -signCorrectedT;
        }

        if (signCorrectedT < 0.0f || signCorrectedT > hitT * abs(det))
        {
            return;
        }
    }

    const float rcpDet = rcp(det);
    bary.x = V * rcpDet;
    bary.y = W * rcpDet;
    hitT = T * rcpDet;
}


static
bool TestLeafNodeIntersections(
    RWByteAddressBufferPointer accelStruct,
    uint2 flags,
    uint instanceFlags,
    float3 rayOrigin,
    float3 rayDirection,
    int3 swizzledIndicies,
    float3 shear,
    inout float2 resultBary,
    inout float resultT,
    inout uint resultTriId)
{

    const uint firstId = GetLeafIndexFromInfo(flags);
    const uint numTris = GetNumPrimitivesFromInfo(flags);


    uint i = 0;
    bool bIsIntersect = false;
#line 352 "dxr-fl/TraverseFunction.hlsli"
    {
        const uint triId0 = firstId + i;


        float3 v0, v1, v2;
        BVHReadTriangle(accelStruct, v0, v1, v2, triId0);


        float2 bary0;
        float t0 = resultT;
        RayTriangleIntersect(
            t0,
            instanceFlags,
            bary0,
            rayOrigin,
            rayDirection,
            swizzledIndicies,
            shear,
            v0, v1, v2);


        if (t0 < resultT && t0 > RayTMin())
        {
            resultBary = bary0.xy;
            resultT = t0;
            resultTriId = triId0;
            bIsIntersect = true;
        }
    }
    return bIsIntersect;
}

int GetIndexOfBiggestChannel(float3 vec)
{
    if (vec.x > vec.y && vec.x > vec.z)
    {
        return 0;
    }
    else if (vec.y > vec.z)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

void swap(inout int a, inout int b)
{
    int temp = a;
    a = b;
    b = temp;
}

struct HitData
{
    uint ContributionToHitGroupIndex;
    uint PrimitiveIndex;
};

struct RayData
{

    float3 InverseDirection;
    float3 OriginTimesRayInverseDirection;
    float3 Shear;
    int3 SwizzledIndices;
};

RayData GetRayData(float3 rayOrigin, float3 rayDirection)
{
    RayData data;


    data.InverseDirection = rcp(rayDirection);
    data.OriginTimesRayInverseDirection = rayOrigin * data.InverseDirection;

    int zIndex = GetIndexOfBiggestChannel(abs(rayDirection));
    data.SwizzledIndices = int3(
        (zIndex + 1) % 3,
        (zIndex + 2) % 3,
        zIndex);

    if (rayDirection[data.SwizzledIndices.z] < 0.0f) swap(data.SwizzledIndices.x, data.SwizzledIndices.y);

    data.Shear = float3(
        rayDirection[data.SwizzledIndices.x] / rayDirection[data.SwizzledIndices.z],
        rayDirection[data.SwizzledIndices.y] / rayDirection[data.SwizzledIndices.z],
        1.0 / rayDirection[data.SwizzledIndices.z]);

    return data;
}

bool Cull(bool opaque, uint rayFlags)
{
    return (opaque && (rayFlags & RAY_FLAG_CULL_OPAQUE)) || (!opaque && (rayFlags & RAY_FLAG_CULL_NON_OPAQUE));
}

float ComputeCullFaceDir(uint instanceFlags, uint rayFlags)
{
    float cullFaceDir = 0;
    if (rayFlags & RAY_FLAG_CULL_FRONT_FACING_TRIANGLES)
        cullFaceDir = 1;
    else if (rayFlags & RAY_FLAG_CULL_BACK_FACING_TRIANGLES)
        cullFaceDir = -1;
    if (instanceFlags & 0x1)
        cullFaceDir = 0;

    return cullFaceDir;
}
#line 471 "dxr-fl/TraverseFunction.hlsli"
void dump(BoundingBox box, uint2 flags)
{
    LogFloat3(box.center);
    LogFloat3(box.halfDim);
    LogInt2(flags);
}





void Fallback_SetPendingAttr(BuiltInTriangleIntersectionAttributes);;




void SetBoolFlag(inout uint flagContainer, uint flag, bool enable)
{
    if (enable)
    {
        flagContainer |= flag;
    }
    else
    {
        flagContainer &= ~flag;
    }
}

bool GetBoolFlag(uint flagContainer, uint flag)
{
    return flagContainer & flag;
}

struct BLASContext {
    uint instanceIndex;
    uint instanceFlags;
    uint instanceOffset;
    uint instanceId;
    uint2 instanceGpuVA;

    float3x4 worldToObject;
    float3x4 objectToWorld;
    float3 objectSpaceOrigin;
    float3 objectSpaceDirection;
    RayData rayData;
};





inline bool GetBLASFromTopLevelLeaf(
    in uint2 leafInfo,
    in RWByteAddressBufferPointer topLevelAccelerationStructure,
    in uint offsetToInstanceDescs,
    in uint InstanceInclusionMask,
    out BLASContext blasContext
)
{
    LogInt(6*100+10+0);

    uint leafIndex = GetLeafIndexFromInfo(leafInfo);

    BVHMetadata metadata = LoadBVHMetadata(topLevelAccelerationStructure.buffer, offsetToInstanceDescs + leafIndex * 116);




    RaytracingInstanceDesc instanceDesc = metadata.instanceDesc;

    bool isValidInstance = GetInstanceMask(instanceDesc) & InstanceInclusionMask;

    if (isValidInstance)
    {
        LogInt(7*100+10+0);

        blasContext.instanceIndex = metadata.InstanceIndex;
        blasContext.instanceOffset = GetInstanceContributionToHitGroupIndex(instanceDesc);
        blasContext.instanceId = GetInstanceID(instanceDesc);

        blasContext.instanceGpuVA = instanceDesc.AccelerationStructure;
        blasContext.instanceFlags = GetInstanceFlags(instanceDesc);

        blasContext.worldToObject = CreateMatrix(instanceDesc.Transform);
        blasContext.objectToWorld = CreateMatrix(metadata.ObjectToWorld);
        blasContext.objectSpaceOrigin = mul(blasContext.worldToObject, float4(WorldRayOrigin(), 1));
        blasContext.objectSpaceDirection = mul(blasContext.worldToObject, float4(WorldRayDirection(), 0));
        blasContext.rayData = GetRayData(blasContext.objectSpaceOrigin, blasContext.objectSpaceDirection);
    }

    return isValidInstance;
}

inline bool CheckHitProcedural(
    in uint hitGroupRecordOffset,
    in uint primitiveIndex,

    in BLASContext blasContext
)
{
    Fallback_SetPendingCustomVals(hitGroupRecordOffset, primitiveIndex, blasContext.instanceIndex, blasContext.instanceId);

    uint intersectionStateId, anyHitStateId;
    GetAnyHitAndIntersectionStateId(HitGroupShaderTable, hitGroupRecordOffset, anyHitStateId, intersectionStateId);

    Fallback_SetAnyHitStateId(anyHitStateId);
    Fallback_SetAnyHitResult(1);
    Fallback_CallIndirect(intersectionStateId);
    return (Fallback_AnyHitResult() == -1);
}

inline bool CheckHitTriangles(
    in uint2 nodeInfo,
    in uint hitGroupRecordOffset,
    in uint primitiveIndex,
    in bool opaque,

    in RWByteAddressBufferPointer bottomLevelAccelerationStructure,

    in BLASContext blasContext,

    in uint searchDepth
)
{
    float resultT = Fallback_RayTCurrent();
    float2 resultBary;
    uint resultTriId;


    bool triangleHit = TestLeafNodeIntersections(
        bottomLevelAccelerationStructure,
        nodeInfo,
        blasContext.instanceFlags,
        ObjectRayOrigin(),
        ObjectRayDirection(),
        blasContext.rayData.SwizzledIndices,
        blasContext.rayData.Shear,
        resultBary,
        resultT,
        resultTriId);

    if (!triangleHit)
    {
        return false;
    }

    uint hitKind = HIT_KIND_TRIANGLE_FRONT_FACE;

    BuiltInTriangleIntersectionAttributes attr;
    attr.barycentrics = resultBary;
    Fallback_SetPendingAttr(attr);

    Fallback_SetPendingTriVals(hitGroupRecordOffset, primitiveIndex, blasContext.instanceIndex, blasContext.instanceId, resultT, hitKind);
#line 636 "dxr-fl/TraverseFunction.hlsli"
    bool skipAnyHit = true;




    bool hitEndsSearch;

    if (skipAnyHit)
    {
        LogInt(8*100+10+1);
        Fallback_CommitHit();
        hitEndsSearch = (RayFlags() & RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH);
    }
    else
    {
        LogInt(8*100+10+2);
        uint anyhitStateId = GetAnyHitStateId(HitGroupShaderTable, hitGroupRecordOffset);
        int ret = 1;

        if (anyhitStateId)
        {
            ret = InvokeAnyHit(anyhitStateId);
        }

        if (ret != 0)
        {
            Fallback_CommitHit();
        }

        hitEndsSearch = (ret == -1) || (RayFlags() & RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH);
    }

    return hitEndsSearch;
}

inline bool CheckHitOnBottomLevelLeaf(
    in uint2 leafInfo,

    in RWByteAddressBufferPointer bottomLevelAccelerationStructure,
    in BLASContext blasContext,

    in uint RayContributionToHitGroupIndex,
    in uint MultiplierForGeometryContributionToHitGroupIndex,

    in uint searchDepth
)
{
    LogInt(8*100+10+0);

    const uint leafIndex = GetLeafIndexFromInfo(leafInfo);
    PrimitiveMetaData primitiveMetadata = BVHReadPrimitiveMetaData(bottomLevelAccelerationStructure, leafIndex);

    bool geomOpaque = primitiveMetadata.GeometryFlags & D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    bool opaque = IsOpaque(geomOpaque, blasContext.instanceFlags, RayFlags());
    bool culled = Cull(opaque, RayFlags());

    bool isProceduralGeometry = IsProceduralGeometry(leafInfo);

    isProceduralGeometry = false;


    if (!culled)
    {
        uint hitGroupGeometryContribution = primitiveMetadata.GeometryContributionToHitGroupIndex * MultiplierForGeometryContributionToHitGroupIndex;
        uint hitGroupRecordIndex = RayContributionToHitGroupIndex + hitGroupGeometryContribution + blasContext.instanceOffset;
        uint hitGroupRecordOffset = HitGroupShaderRecordStride * hitGroupRecordIndex;

        uint primitiveIndex = primitiveMetadata.PrimitiveIndex;
        if (isProceduralGeometry)
        {
            return CheckHitProcedural(
                hitGroupRecordOffset,
                primitiveIndex,
                blasContext
            );
        }
        else
        {
            return CheckHitTriangles(
                leafInfo,
                hitGroupRecordOffset,
                primitiveIndex,
                opaque,
                bottomLevelAccelerationStructure,
                blasContext,
                searchDepth
            );
        }
    }

    return false;
}

static
bool Traverse(
    uint InstanceInclusionMask,
    uint RayContributionToHitGroupIndex,
    uint MultiplierForGeometryContributionToHitGroupIndex
)
{
    uint GI = Fallback_GroupIndex();

    RayData currentRayData = GetRayData(WorldRayOrigin(), WorldRayDirection());

    uint nodesToProcess[2];
    nodesToProcess[0] = 0;
    nodesToProcess[1] = 0;
    uint currentBVHLevel = 0;

    BLASContext blasContext;

    RWByteAddressBufferPointer topLevelAccelerationStructure = CreateRWByteAddressBufferPointerFromGpuVA(TopLevelAccelerationStructureGpuVA);
    uint offsetToInstanceDescs = topLevelAccelerationStructure.buffer.Load(OffsetToLeafNodeMetaDataOffset + topLevelAccelerationStructure.offsetInBytes) + topLevelAccelerationStructure.offsetInBytes;
    RWByteAddressBufferPointer currentBVH = topLevelAccelerationStructure;

    float closestBoxT = asfloat(0x7F7FFFFF);
    int NO_HIT_SENTINEL = ~0;
    Fallback_SetInstanceIndex(NO_HIT_SENTINEL);

    uint currentDepth = 0;

    uint2 rootInfo = 0;
    StackPush(nodesToProcess[0], 0, rootInfo, currentDepth);

    uint moo = 0;
    LogInt(2*100+10+0);
    do
    {
        LogInt(3*100+10+0);
        uint2 parentNodeInfo = StackPop(nodesToProcess[currentBVHLevel], currentBVHLevel, currentDepth);
        bool isLeaf = IsLeaf(parentNodeInfo);
        uint childIndex = GetChildIndexFromInfo(parentNodeInfo);

        if (isLeaf)
        {
            LogInt(5*100+10+0);
            if (currentBVHLevel == 0)
            {
                LogInt(6*100+10+0);
                if (GetBLASFromTopLevelLeaf(
                    parentNodeInfo,
                    topLevelAccelerationStructure,
                    offsetToInstanceDescs,
                    InstanceInclusionMask,
                    blasContext
                ))
                {
                    LogInt(7*100+10+0);

                    currentRayData = blasContext.rayData;
                    currentBVH = CreateRWByteAddressBufferPointerFromGpuVA(blasContext.instanceGpuVA);

                    UpdateObjectSpaceProperties(
                        blasContext.objectSpaceOrigin,
                        blasContext.objectSpaceDirection,
                        blasContext.worldToObject,
                        blasContext.objectToWorld
                    );

                    StackPush(nodesToProcess[1], 1, rootInfo, currentDepth + 1);
                    currentBVHLevel = 1;

                }
            }
            else
            {
                if(CheckHitOnBottomLevelLeaf(
                    parentNodeInfo,
                    currentBVH,
                    blasContext,
                    RayContributionToHitGroupIndex,
                    MultiplierForGeometryContributionToHitGroupIndex,
                    currentDepth
                ))
                {
                    break;
                }

            }
        }
        else
        {
            moo++;
            LogInt(4*100+10+0);
            BoundingBox leftBox, rightBox;
            uint2 leftInfo, rightInfo;
            bool leftHit, rightHit;
            float leftT, rightT;

            leftBox = GetLeftBoxFromBVH(currentBVH, childIndex, leftInfo);
            rightBox = GetRightBoxFromBVH(currentBVH, childIndex, rightInfo);

            leftHit = RayBoxTest(
                leftT,
                RayTCurrent(),
                currentRayData.OriginTimesRayInverseDirection,
                currentRayData.InverseDirection,
                leftBox.center,
                leftBox.halfDim);

            rightHit = !IsDummy(rightInfo) && RayBoxTest(
                rightT,
                RayTCurrent(),
                currentRayData.OriginTimesRayInverseDirection,
                currentRayData.InverseDirection,
                rightBox.center,
                rightBox.halfDim);

            bool singleHit, doubleHit;

            singleHit = leftHit || rightHit;
            doubleHit = leftHit && rightHit;

            uint2 firstInfo, secondInfo;

            if (doubleHit)
            {
                if (rightT < leftT)
                {
                    firstInfo = rightInfo; secondInfo = leftInfo;
                }
                else
                {
                    firstInfo = leftInfo; secondInfo = rightInfo;
                }
            }
            else if (singleHit)
            {
                firstInfo = leftHit ? leftInfo : rightInfo;
            }

            if (doubleHit)
            {
                StackPush(nodesToProcess[currentBVHLevel], currentBVHLevel, secondInfo, currentDepth + 1);
            }

            if (singleHit)
            {
                StackPush(nodesToProcess[currentBVHLevel], currentBVHLevel, firstInfo, currentDepth + 1);
            }
        }

        if (nodesToProcess[1] == 0)
        {
            if (currentBVHLevel == 1)
            {
                currentBVHLevel = 0;
                currentRayData = GetRayData(WorldRayOrigin(), WorldRayDirection());
                currentBVH = topLevelAccelerationStructure;
            }
        }
    } while(nodesToProcess[currentBVHLevel] != 0);
    LogInt(10*100+10+0);

    bool isHit = Fallback_InstanceIndex() != NO_HIT_SENTINEL;




    return isHit;
}
#line 19 "dxr-fl/TraverseShader.hlsli"


[experimental("shader", "internal")]
void Fallback_TraceRay(
    uint rayFlags,
    uint instanceInclusionMask,
    uint rayContributionToHitGroupIndex,
    uint multiplierForGeometryContributionToHitGroupIndex,
    uint missShaderIndex,
    float originX,
    float originY,
    float originZ,
    float tMin,
    float directionX,
    float directionY,
    float directionZ,
    float tMax,
    uint payloadOffset)
{
    LogTraceRayStart();
    uint oldPayloadOffset = Fallback_TraceRayBegin(rayFlags, float3(originX, originY, originZ), tMin, float3(directionX, directionY, directionZ), tMax, payloadOffset);

    bool hit = Traverse(
        instanceInclusionMask,
        rayContributionToHitGroupIndex,
        multiplierForGeometryContributionToHitGroupIndex
    );

    uint stateID;
    if (hit)
    {
      if (RayFlags() & RAY_FLAG_SKIP_CLOSEST_HIT_SHADER)
      {
        stateID = 0;
      }
      else
      {
        stateID = HitGroupShaderTable.Load(Fallback_ShaderRecordOffset());
      }
    }
    else
    {
      int missShaderRecordOffset = missShaderIndex * MissShaderRecordStride;
      Fallback_SetShaderRecordOffset(missShaderRecordOffset);
      stateID = MissShaderTable.Load(missShaderRecordOffset);
    }

    if (stateID != 0)
    {
        Fallback_CallIndirect(stateID);
    }

    Fallback_TraceRayEnd(oldPayloadOffset);
    LogTraceRayEnd();
}
#line 13 "dxr-fl\\MinimalTraverseShaderLib.hlsl"

