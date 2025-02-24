//
// main_count
//
struct main_count_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[3];
};
RWByteAddressBuffer indices : register(u10);
RWByteAddressBuffer positions : register(u11);
RWByteAddressBuffer counters : register(u20);
RWByteAddressBuffer LUT : register(u21);
RWByteAddressBuffer dbg : register(u50);
float3 toVoxelPos(float3 position) {
  float3 bbMin = float3(asfloat(uniforms[1u].x), asfloat(uniforms[1u].y), asfloat(uniforms[1u].z));
  float3 bbMax = float3(asfloat(uniforms[2u].x), asfloat(uniforms[2u].y), asfloat(uniforms[2u].z));
  float3 bbSize = (bbMax - bbMin);
  float cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  float gridSize = float(uniforms[0u].y);
  float gx = ((gridSize * (position.x - asfloat(uniforms[1u].x))) / cubeSize);
  float gy = ((gridSize * (position.y - asfloat(uniforms[1u].y))) / cubeSize);
  float gz = ((gridSize * (position.z - asfloat(uniforms[1u].z))) / cubeSize);
  return float3(gx, gy, gz);
}

uint3 tint_v3f32_to_v3u32(float3 value) {
  return (((value <= (4294967040.0f).xxx)) ? ((((value >= (0.0f).xxx)) ? (uint3(value)) : ((0u).xxx))) : ((4294967295u).xxx));
}

uint toIndex1D(uint gridSize, float3 voxelPos) {
  uint3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

float3 loadPosition(uint vertexIndex) {
  uint v = 0u;
  positions.GetDimensions(v);
  uint v_1 = 0u;
  positions.GetDimensions(v_1);
  uint v_2 = 0u;
  positions.GetDimensions(v_2);
  float3 position = float3(asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 0u), ((v / 4u) - 1u)) * 4u)))), asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 1u), ((v_1 / 4u) - 1u)) * 4u)))), asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 2u), ((v_2 / 4u) - 1u)) * 4u)))));
  return position;
}

void doIgnore() {
  uint g42 = uniforms[0u].x;
  uint kj6 = dbg.Load(20u);
  uint v_3 = 0u;
  counters.GetDimensions(v_3);
  uint v_4 = ((v_3 / 4u) - 1u);
  uint v_5 = 0u;
  counters.InterlockedOr((0u + (min(uint(int(0)), v_4) * 4u)), 0u, v_5);
  uint b53 = v_5;
  uint v_6 = 0u;
  indices.GetDimensions(v_6);
  uint v_7 = ((v_6 / 4u) - 1u);
  uint rwg = indices.Load((0u + (min(uint(int(0)), v_7) * 4u)));
  uint v_8 = 0u;
  positions.GetDimensions(v_8);
  uint v_9 = ((v_8 / 4u) - 1u);
  float rb5 = asfloat(positions.Load((0u + (min(uint(int(0)), v_9) * 4u))));
  uint v_10 = 0u;
  LUT.GetDimensions(v_10);
  uint v_11 = ((v_10 / 4u) - 1u);
  int v_12 = int(0);
  LUT.InterlockedOr(int((0u + (min(uint(int(0)), v_11) * 4u))), int(0), v_12);
  int g55 = v_12;
}

void main_count_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms[0u].x)) {
    return;
  }
  doIgnore();
  uint v_13 = 0u;
  indices.GetDimensions(v_13);
  uint i0 = indices.Load((0u + (min(((3u * triangleIndex) + 0u), ((v_13 / 4u) - 1u)) * 4u)));
  uint v_14 = 0u;
  indices.GetDimensions(v_14);
  uint i1 = indices.Load((0u + (min(((3u * triangleIndex) + 1u), ((v_14 / 4u) - 1u)) * 4u)));
  uint v_15 = 0u;
  indices.GetDimensions(v_15);
  uint i2 = indices.Load((0u + (min(((3u * triangleIndex) + 2u), ((v_15 / 4u) - 1u)) * 4u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i1);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p1) + p2) / 3.0f);
  float3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms[0u].y, voxelPos);
  uint v_16 = 0u;
  counters.GetDimensions(v_16);
  uint v_17 = 0u;
  counters.InterlockedAdd((0u + (min(voxelIndex, ((v_16 / 4u) - 1u)) * 4u)), 1u, v_17);
  uint acefg = v_17;
  if ((triangleIndex == 0u)) {
    dbg.Store(16u, uniforms[0u].y);
    dbg.Store(32u, asuint(center.x));
    dbg.Store(36u, asuint(center.y));
    dbg.Store(40u, asuint(center.z));
  }
}

[numthreads(128, 1, 1)]
void main_count(main_count_inputs inputs) {
  main_count_inner(inputs.GlobalInvocationID);
}

//
// main_create_lut
//
struct main_create_lut_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[3];
};
RWByteAddressBuffer indices : register(u10);
RWByteAddressBuffer positions : register(u11);
RWByteAddressBuffer counters : register(u20);
RWByteAddressBuffer LUT : register(u21);
RWByteAddressBuffer dbg : register(u50);
void doIgnore() {
  uint g42 = uniforms[0u].x;
  uint kj6 = dbg.Load(20u);
  uint v = 0u;
  counters.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  uint v_2 = 0u;
  counters.InterlockedOr((0u + (min(uint(int(0)), v_1) * 4u)), 0u, v_2);
  uint b53 = v_2;
  uint v_3 = 0u;
  indices.GetDimensions(v_3);
  uint v_4 = ((v_3 / 4u) - 1u);
  uint rwg = indices.Load((0u + (min(uint(int(0)), v_4) * 4u)));
  uint v_5 = 0u;
  positions.GetDimensions(v_5);
  uint v_6 = ((v_5 / 4u) - 1u);
  float rb5 = asfloat(positions.Load((0u + (min(uint(int(0)), v_6) * 4u))));
  uint v_7 = 0u;
  LUT.GetDimensions(v_7);
  uint v_8 = ((v_7 / 4u) - 1u);
  int v_9 = int(0);
  LUT.InterlockedOr(int((0u + (min(uint(int(0)), v_8) * 4u))), int(0), v_9);
  int g55 = v_9;
}

void main_create_lut_inner(uint3 GlobalInvocationID) {
  uint voxelIndex = GlobalInvocationID.x;
  doIgnore();
  uint maxVoxels = ((uniforms[0u].y * uniforms[0u].y) * uniforms[0u].y);
  if ((voxelIndex >= maxVoxels)) {
    return;
  }
  uint v_10 = 0u;
  counters.GetDimensions(v_10);
  uint v_11 = 0u;
  counters.InterlockedOr((0u + (min(voxelIndex, ((v_10 / 4u) - 1u)) * 4u)), 0u, v_11);
  uint numTriangles = v_11;
  int offset = int(-1);
  if ((numTriangles > 0u)) {
    uint v_12 = 0u;
    dbg.InterlockedAdd(0u, numTriangles, v_12);
    offset = int(v_12);
  }
  uint v_13 = 0u;
  LUT.GetDimensions(v_13);
  int v_14 = offset;
  int v_15 = int(0);
  LUT.InterlockedExchange(int((0u + (min(voxelIndex, ((v_13 / 4u) - 1u)) * 4u))), v_14, v_15);
}

[numthreads(128, 1, 1)]
void main_create_lut(main_create_lut_inputs inputs) {
  main_create_lut_inner(inputs.GlobalInvocationID);
}

//
// main_sort_triangles
//
struct main_sort_triangles_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[3];
};
RWByteAddressBuffer indices : register(u10);
RWByteAddressBuffer positions : register(u11);
RWByteAddressBuffer counters : register(u20);
RWByteAddressBuffer LUT : register(u21);
RWByteAddressBuffer dbg : register(u50);
float3 toVoxelPos(float3 position) {
  float3 bbMin = float3(asfloat(uniforms[1u].x), asfloat(uniforms[1u].y), asfloat(uniforms[1u].z));
  float3 bbMax = float3(asfloat(uniforms[2u].x), asfloat(uniforms[2u].y), asfloat(uniforms[2u].z));
  float3 bbSize = (bbMax - bbMin);
  float cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  float gridSize = float(uniforms[0u].y);
  float gx = ((gridSize * (position.x - asfloat(uniforms[1u].x))) / cubeSize);
  float gy = ((gridSize * (position.y - asfloat(uniforms[1u].y))) / cubeSize);
  float gz = ((gridSize * (position.z - asfloat(uniforms[1u].z))) / cubeSize);
  return float3(gx, gy, gz);
}

uint3 tint_v3f32_to_v3u32(float3 value) {
  return (((value <= (4294967040.0f).xxx)) ? ((((value >= (0.0f).xxx)) ? (uint3(value)) : ((0u).xxx))) : ((4294967295u).xxx));
}

uint toIndex1D(uint gridSize, float3 voxelPos) {
  uint3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

float3 loadPosition(uint vertexIndex) {
  uint v = 0u;
  positions.GetDimensions(v);
  uint v_1 = 0u;
  positions.GetDimensions(v_1);
  uint v_2 = 0u;
  positions.GetDimensions(v_2);
  float3 position = float3(asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 0u), ((v / 4u) - 1u)) * 4u)))), asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 1u), ((v_1 / 4u) - 1u)) * 4u)))), asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 2u), ((v_2 / 4u) - 1u)) * 4u)))));
  return position;
}

void doIgnore() {
  uint g42 = uniforms[0u].x;
  uint kj6 = dbg.Load(20u);
  uint v_3 = 0u;
  counters.GetDimensions(v_3);
  uint v_4 = ((v_3 / 4u) - 1u);
  uint v_5 = 0u;
  counters.InterlockedOr((0u + (min(uint(int(0)), v_4) * 4u)), 0u, v_5);
  uint b53 = v_5;
  uint v_6 = 0u;
  indices.GetDimensions(v_6);
  uint v_7 = ((v_6 / 4u) - 1u);
  uint rwg = indices.Load((0u + (min(uint(int(0)), v_7) * 4u)));
  uint v_8 = 0u;
  positions.GetDimensions(v_8);
  uint v_9 = ((v_8 / 4u) - 1u);
  float rb5 = asfloat(positions.Load((0u + (min(uint(int(0)), v_9) * 4u))));
  uint v_10 = 0u;
  LUT.GetDimensions(v_10);
  uint v_11 = ((v_10 / 4u) - 1u);
  int v_12 = int(0);
  LUT.InterlockedOr(int((0u + (min(uint(int(0)), v_11) * 4u))), int(0), v_12);
  int g55 = v_12;
}

void main_sort_triangles_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  doIgnore();
  if ((triangleIndex >= uniforms[0u].x)) {
    return;
  }
  uint v_13 = 0u;
  indices.GetDimensions(v_13);
  uint i0 = indices.Load((0u + (min(((3u * triangleIndex) + 0u), ((v_13 / 4u) - 1u)) * 4u)));
  uint v_14 = 0u;
  indices.GetDimensions(v_14);
  uint i1 = indices.Load((0u + (min(((3u * triangleIndex) + 1u), ((v_14 / 4u) - 1u)) * 4u)));
  uint v_15 = 0u;
  indices.GetDimensions(v_15);
  uint i2 = indices.Load((0u + (min(((3u * triangleIndex) + 2u), ((v_15 / 4u) - 1u)) * 4u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i1);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p1) + p2) / 3.0f);
  float3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms[0u].y, voxelPos);
  uint v_16 = 0u;
  LUT.GetDimensions(v_16);
  int v_17 = int(0);
  LUT.InterlockedAdd(int((0u + (min(voxelIndex, ((v_16 / 4u) - 1u)) * 4u))), int(1), v_17);
  int triangleOffset = v_17;
}

[numthreads(128, 1, 1)]
void main_sort_triangles(main_sort_triangles_inputs inputs) {
  main_sort_triangles_inner(inputs.GlobalInvocationID);
}

