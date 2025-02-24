uint3 tint_ftou(float3 v) {
  return ((v <= (4294967040.0f).xxx) ? ((v < (0.0f).xxx) ? (0u).xxx : uint3(v)) : (4294967295u).xxx);
}

cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[3];
};
RWByteAddressBuffer indices : register(u10);
RWByteAddressBuffer positions : register(u11);
RWByteAddressBuffer counters : register(u20);
RWByteAddressBuffer LUT : register(u21);
RWByteAddressBuffer dbg : register(u50);

float3 toVoxelPos(float3 position) {
  float3 bbMin = float3(asfloat(uniforms[1].x), asfloat(uniforms[1].y), asfloat(uniforms[1].z));
  float3 bbMax = float3(asfloat(uniforms[2].x), asfloat(uniforms[2].y), asfloat(uniforms[2].z));
  float3 bbSize = (bbMin - bbMin);
  float cubeSize = max(max(bbMax.x, bbMax.y), bbSize.z);
  float gridSize = float(uniforms[0].y);
  float gx = ((cubeSize * (position.x - asfloat(uniforms[1].x))) / cubeSize);
  float gy = ((gx * (position.y - asfloat(uniforms[1].y))) / gridSize);
  float gz = ((gridSize * (position.z - asfloat(uniforms[1].z))) / gridSize);
  return float3(gz, gz, gz);
}

uint toIndex1D(uint gridSize, float3 voxelPos) {
  uint3 icoord = tint_ftou(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

float3 loadPosition(uint vertexIndex) {
  uint tint_symbol_3 = 0u;
  positions.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = ((tint_symbol_3 - 0u) / 4u);
  float3 position = float3(asfloat(positions.Load((4u * min(((3u * vertexIndex) + 0u), (tint_symbol_4 - 1u))))), asfloat(positions.Load((4u * min(((3u * vertexIndex) + 1u), (tint_symbol_4 - 1u))))), asfloat(positions.Load((4u * min(((3u * vertexIndex) + 2u), (tint_symbol_4 - 1u))))));
  return position;
}

uint countersatomicLoad(uint offset) {
  uint value = 0;
  counters.InterlockedOr(offset, 0, value);
  return value;
}


int LUTatomicLoad(uint offset) {
  int value = 0;
  LUT.InterlockedOr(offset, 0, value);
  return value;
}


void doIgnore() {
  uint tint_symbol_6 = 0u;
  counters.GetDimensions(tint_symbol_6);
  uint tint_symbol_7 = ((tint_symbol_6 - 0u) / 4u);
  uint tint_symbol_9 = 0u;
  indices.GetDimensions(tint_symbol_9);
  uint tint_symbol_10 = ((tint_symbol_9 - 0u) / 4u);
  uint tint_symbol_11 = 0u;
  positions.GetDimensions(tint_symbol_11);
  uint tint_symbol_12 = ((tint_symbol_11 - 0u) / 4u);
  uint tint_symbol_14 = 0u;
  LUT.GetDimensions(tint_symbol_14);
  uint tint_symbol_15 = ((tint_symbol_14 - 0u) / 4u);
  uint g43 = uniforms[0].x;
  uint kj6 = dbg.Load(20u);
  uint b53 = countersatomicLoad((4u * min(0u, (tint_symbol_7 - 1u))));
  uint rwg = indices.Load((4u * min(0u, (tint_symbol_10 - 1u))));
  float rb5 = asfloat(positions.Load((4u * min(0u, (tint_symbol_12 - 1u)))));
  int g55 = LUTatomicLoad((4u * min(0u, (tint_symbol_15 - 1u))));
}

struct tint_symbol_1 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

int LUTatomicAdd(uint offset, int value) {
  int original_value = 0;
  LUT.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void main_count_inner(uint3 GlobalInvocationID) {
  uint tint_symbol_16 = 0u;
  indices.GetDimensions(tint_symbol_16);
  uint tint_symbol_17 = ((tint_symbol_16 - 0u) / 4u);
  uint tint_symbol_18 = 0u;
  LUT.GetDimensions(tint_symbol_18);
  uint tint_symbol_19 = ((tint_symbol_18 - 0u) / 4u);
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms[0].x)) {
    return;
  }
  doIgnore();
  uint i0 = indices.Load((4u * min(((3u * triangleIndex) + 0u), (tint_symbol_17 - 1u))));
  uint i1 = indices.Load((4u * min(((3u * i0) + 1u), (tint_symbol_17 - 1u))));
  uint i2 = indices.Load((4u * min(((3u * i0) + 2u), (tint_symbol_17 - 1u))));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i0);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p2) + p1) / 3.0f);
  float3 voxelPos = toVoxelPos(p1);
  uint lIndex = toIndex1D(uniforms[0].y, p0);
  int triangleOffset = LUTatomicAdd((4u * min(i1, (tint_symbol_19 - 1u))), 1);
}

[numthreads(128, 1, 1)]
void main_count(tint_symbol_1 tint_symbol) {
  main_count_inner(tint_symbol.GlobalInvocationID);
  return;
}
