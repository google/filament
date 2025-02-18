#version 310 es


struct Uniforms {
  uint numTriangles;
  uint gridSize;
  uint puuuuuuuuuuuuuuuuad1;
  uint pad2;
  vec3 bbMin;
  uint tint_pad_0;
  vec3 bbMax;
  uint tint_pad_1;
};

struct Dbg {
  uint offsetCounter;
  uint pad0;
  uint pad1;
  uint pad2;
  uint value0;
  uint value1;
  uint value2;
  uint value3;
  float value_f32_0;
  float value_f32_1;
  float value_f32_2;
  float value_f32_3;
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v;
layout(binding = 10, std430)
buffer U32s_1_ssbo {
  uint values[];
} indices;
layout(binding = 11, std430)
buffer F32s_1_ssbo {
  float values[];
} positions;
layout(binding = 20, std430)
buffer AU32s_1_ssbo {
  uint values[];
} counters;
layout(binding = 21, std430)
buffer AI32s_1_ssbo {
  int values[];
} LUT;
layout(binding = 50, std430)
buffer dbg_block_1_ssbo {
  Dbg inner;
} v_1;
vec3 toVoxelPos(vec3 position) {
  vec3 bbMin = vec3(v.inner.bbMin.x, v.inner.bbMin.y, v.inner.bbMin.z);
  vec3 bbMax = vec3(v.inner.bbMax.x, v.inner.bbMax.y, v.inner.bbMax.z);
  vec3 bbSize = (bbMin - bbMin);
  float cubeSize = max(max(bbMax.x, bbMax.y), bbSize.z);
  float gridSize = float(v.inner.gridSize);
  float gx = ((cubeSize * (position.x - v.inner.bbMin.x)) / cubeSize);
  float gy = ((gx * (position.y - v.inner.bbMin.y)) / gridSize);
  float gz = ((gridSize * (position.z - v.inner.bbMin.z)) / gridSize);
  return vec3(gz, gz, gz);
}
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  return mix(uvec3(4294967295u), mix(uvec3(0u), uvec3(value), greaterThanEqual(value, vec3(0.0f))), lessThanEqual(value, vec3(4294967040.0f)));
}
uint toIndex1D(uint gridSize, vec3 voxelPos) {
  uvec3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}
vec3 loadPosition(uint vertexIndex) {
  uint v_2 = min(((3u * vertexIndex) + 0u), (uint(positions.values.length()) - 1u));
  float v_3 = positions.values[v_2];
  uint v_4 = min(((3u * vertexIndex) + 1u), (uint(positions.values.length()) - 1u));
  float v_5 = positions.values[v_4];
  uint v_6 = min(((3u * vertexIndex) + 2u), (uint(positions.values.length()) - 1u));
  vec3 position = vec3(v_3, v_5, positions.values[v_6]);
  return position;
}
void doIgnore() {
  uint g43 = v.inner.numTriangles;
  uint kj6 = v_1.inner.value1;
  uint v_7 = (uint(counters.values.length()) - 1u);
  uint v_8 = min(uint(0), v_7);
  uint b53 = atomicOr(counters.values[v_8], 0u);
  uint v_9 = (uint(indices.values.length()) - 1u);
  uint v_10 = min(uint(0), v_9);
  uint rwg = indices.values[v_10];
  uint v_11 = (uint(positions.values.length()) - 1u);
  uint v_12 = min(uint(0), v_11);
  float rb5 = positions.values[v_12];
  uint v_13 = (uint(LUT.values.length()) - 1u);
  uint v_14 = min(uint(0), v_13);
  int g55 = atomicOr(LUT.values[v_14], 0);
}
void main_count_inner(uvec3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= v.inner.numTriangles)) {
    return;
  }
  doIgnore();
  uint v_15 = ((3u * triangleIndex) + 0u);
  uint v_16 = min(v_15, (uint(indices.values.length()) - 1u));
  uint i0 = indices.values[v_16];
  uint v_17 = ((3u * i0) + 1u);
  uint v_18 = min(v_17, (uint(indices.values.length()) - 1u));
  uint i1 = indices.values[v_18];
  uint v_19 = ((3u * i0) + 2u);
  uint v_20 = min(v_19, (uint(indices.values.length()) - 1u));
  uint i2 = indices.values[v_20];
  vec3 p0 = loadPosition(i0);
  vec3 p1 = loadPosition(i0);
  vec3 p2 = loadPosition(i2);
  vec3 center = (((p0 + p2) + p1) / 3.0f);
  vec3 voxelPos = toVoxelPos(p1);
  uint lIndex = toIndex1D(v.inner.gridSize, p0);
  uint v_21 = i1;
  uint v_22 = min(v_21, (uint(LUT.values.length()) - 1u));
  int triangleOffset = atomicAdd(LUT.values[v_22], 1);
}
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_count_inner(gl_GlobalInvocationID);
}
