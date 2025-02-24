struct Uniforms {
  numTriangles : u32,
  gridSize : u32,
  pad1 : u32,
  pad2 : u32,
  bbMin : vec3<f32>,
  bbMax : vec3<f32>,
}

struct Dbg {
  offsetCounter : atomic<u32>,
  pad0 : u32,
  pad1 : u32,
  pad2 : u32,
  value0 : u32,
  value1 : u32,
  value2 : u32,
  value3 : u32,
  value_f32_0 : f32,
  value_f32_1 : f32,
  value_f32_2 : f32,
  value_f32_3 : f32,
}

struct F32s {
  values : array<f32>,
}

struct U32s {
  values : array<u32>,
}

struct I32s {
  values : array<i32>,
}

struct AU32s {
  values : array<atomic<u32>>,
}

struct AI32s {
  values : array<atomic<i32>>,
}

@binding(0) @group(0) var<uniform> uniforms : Uniforms;

@binding(10) @group(0) var<storage, read_write> indices : U32s;

@binding(11) @group(0) var<storage, read_write> positions : F32s;

@binding(20) @group(0) var<storage, read_write> counters : AU32s;

@binding(21) @group(0) var<storage, read_write> LUT : AI32s;

@binding(50) @group(0) var<storage, read_write> dbg : Dbg;

fn toVoxelPos(position : vec3<f32>) -> vec3<f32> {
  var bbMin = vec3<f32>(uniforms.bbMin.x, uniforms.bbMin.y, uniforms.bbMin.z);
  var bbMax = vec3<f32>(uniforms.bbMax.x, uniforms.bbMax.y, uniforms.bbMax.z);
  var bbSize = (bbMax - bbMin);
  var cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  var gridSize = f32(uniforms.gridSize);
  var gx = ((gridSize * (position.x - uniforms.bbMin.x)) / cubeSize);
  var gy = ((gridSize * (position.y - uniforms.bbMin.y)) / cubeSize);
  var gz = ((gridSize * (position.z - uniforms.bbMin.z)) / cubeSize);
  return vec3<f32>(gx, gy, gz);
}

fn toIndex1D(gridSize : u32, voxelPos : vec3<f32>) -> u32 {
  var icoord = vec3<u32>(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

fn toIndex3D(gridSize : u32, index : u32) -> vec3<u32> {
  var z = (index / (gridSize * gridSize));
  var y = ((index - ((gridSize * gridSize) * z)) / gridSize);
  var x = (index % gridSize);
  return vec3<u32>(x, y, z);
}

fn loadPosition(vertexIndex : u32) -> vec3<f32> {
  var position = vec3<f32>(positions.values[((3u * vertexIndex) + 0u)], positions.values[((3u * vertexIndex) + 1u)], positions.values[((3u * vertexIndex) + 2u)]);
  return position;
}

fn doIgnore() {
  var g42 = uniforms.numTriangles;
  var kj6 = dbg.value1;
  var b53 = atomicLoad(&(counters.values[0]));
  var rwg = indices.values[0];
  var rb5 = positions.values[0];
  var g55 = atomicLoad(&(LUT.values[0]));
}

@compute @workgroup_size(128)
fn main_count(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  var triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms.numTriangles)) {
    return;
  }
  doIgnore();
  var i0 = indices.values[((3u * triangleIndex) + 0u)];
  var i1 = indices.values[((3u * triangleIndex) + 1u)];
  var i2 = indices.values[((3u * triangleIndex) + 2u)];
  var p0 = loadPosition(i0);
  var p1 = loadPosition(i1);
  var p2 = loadPosition(i2);
  var center = (((p0 + p1) + p2) / 3.0);
  var voxelPos = toVoxelPos(center);
  var voxelIndex = toIndex1D(uniforms.gridSize, voxelPos);
  var acefg = atomicAdd(&(counters.values[voxelIndex]), 1u);
  if ((triangleIndex == 0u)) {
    dbg.value0 = uniforms.gridSize;
    dbg.value_f32_0 = center.x;
    dbg.value_f32_1 = center.y;
    dbg.value_f32_2 = center.z;
  }
}

@compute @workgroup_size(128)
fn main_create_lut(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  var voxelIndex = GlobalInvocationID.x;
  doIgnore();
  var maxVoxels = ((uniforms.gridSize * uniforms.gridSize) * uniforms.gridSize);
  if ((voxelIndex >= maxVoxels)) {
    return;
  }
  var numTriangles = atomicLoad(&(counters.values[voxelIndex]));
  var offset = -(1);
  if ((numTriangles > 0u)) {
    offset = i32(atomicAdd(&(dbg.offsetCounter), numTriangles));
  }
  atomicStore(&(LUT.values[voxelIndex]), offset);
}

@compute @workgroup_size(128)
fn main_sort_triangles(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  var triangleIndex = GlobalInvocationID.x;
  doIgnore();
  if ((triangleIndex >= uniforms.numTriangles)) {
    return;
  }
  var i0 = indices.values[((3u * triangleIndex) + 0u)];
  var i1 = indices.values[((3u * triangleIndex) + 1u)];
  var i2 = indices.values[((3u * triangleIndex) + 2u)];
  var p0 = loadPosition(i0);
  var p1 = loadPosition(i1);
  var p2 = loadPosition(i2);
  var center = (((p0 + p1) + p2) / 3.0);
  var voxelPos = toVoxelPos(center);
  var voxelIndex = toIndex1D(uniforms.gridSize, voxelPos);
  var triangleOffset = atomicAdd(&(LUT.values[voxelIndex]), 1);
}
