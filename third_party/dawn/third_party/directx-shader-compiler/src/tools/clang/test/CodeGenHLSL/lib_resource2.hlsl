// Check resource link with lib_cs_entry.hlsl

RWStructuredBuffer<float2x2> unusedBuf;

void UsedResFn(float2x2  m, uint gidx) {
  unusedBuf[gidx] = m;
}

RWStructuredBuffer<float2x2> fA;

void StoreOutputMat(float2x2  m, uint gidx) {
  fA[gidx] = m;
}

void StoreCSOutput(uint2 tid, uint2 gid) {
  fA[gid.x][gid.y] = tid;
}

struct mat {
  row_major float2x2 f2x2;
};

StructuredBuffer<mat> mats;
StructuredBuffer<row_major float2x2> mats2;

float2x2 LoadInputMat(uint x, uint y) {
  return mats.Load(x).f2x2 + mats2.Load(y);
}

cbuffer B {
  float b;
}

groupshared column_major float2x2 dataC[8*8];

float2x2 RotateMat(float2x2 m, uint x, uint y) {
    dataC[x%(8*8)] = m;
    GroupMemoryBarrierWithGroupSync();
    float2x2 f2x2 = dataC[8*8-1-y%(8*8)];
    return f2x2 + b;
}