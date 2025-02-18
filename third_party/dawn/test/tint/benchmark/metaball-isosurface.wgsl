struct Tables {
  edges : array<u32, 256>,
  tris : array<i32, 4096>,
}

@group(0) @binding(0) var<storage> tables : Tables;

struct IsosurfaceVolume {
  min : vec3<f32>,
  max : vec3<f32>,
  step : vec3<f32>,
  size : vec3<u32>,
  threshold : f32,
  values : array<f32>,
}

@group(0) @binding(1) var<storage, read_write> volume : IsosurfaceVolume;

struct PositionBuffer {
  values : array<f32>,
}

@group(0) @binding(2) var<storage, read_write> positionsOut : PositionBuffer;

struct NormalBuffer {
  values : array<f32>,
}

@group(0) @binding(3) var<storage, read_write> normalsOut : NormalBuffer;

struct IndexBuffer {
  tris : array<u32>,
}

@group(0) @binding(4) var<storage, read_write> indicesOut : IndexBuffer;

struct DrawIndirectArgs {
  vc : u32,
  vertexCount : atomic<u32>,
  firstVertex : u32,
  firstInstance : u32,
  indexCount : atomic<u32>,
  indexedInstanceCount : u32,
  indexedFirstIndex : u32,
  indexedBaseVertex : u32,
  indexedFirstInstance : u32,
}

@group(0) @binding(5) var<storage, read_write> drawOut : DrawIndirectArgs;

fn valueAt(index : vec3<u32>) -> f32 {
  if (any((index >= volume.size))) {
    return 0.0;
  }
  let valueIndex = ((index.x + (index.y * volume.size.x)) + ((index.z * volume.size.x) * volume.size.y));
  return volume.values[valueIndex];
}

fn positionAt(index : vec3<u32>) -> vec3<f32> {
  return (volume.min + (volume.step * vec3<f32>(index.xyz)));
}

fn normalAt(index : vec3<u32>) -> vec3<f32> {
  return vec3<f32>((valueAt((index - vec3<u32>(1u, 0u, 0u))) - valueAt((index + vec3<u32>(1u, 0u, 0u)))), (valueAt((index - vec3<u32>(0u, 1u, 0u))) - valueAt((index + vec3<u32>(0u, 1u, 0u)))), (valueAt((index - vec3<u32>(0u, 0u, 1u))) - valueAt((index + vec3<u32>(0u, 0u, 1u)))));
}

var<private> positions : array<vec3<f32>, 12>;

var<private> normals : array<vec3<f32>, 12>;

var<private> indices : array<u32, 12>;

var<private> cubeVerts : u32 = 0u;

fn interpX(index : u32, i : vec3<u32>, va : f32, vb : f32) {
  let mu = ((volume.threshold - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + vec3<f32>((volume.step.x * mu), 0.0, 0.0));
  let na = normalAt(i);
  let nb = normalAt((i + vec3<u32>(1u, 0u, 0u)));
  normals[cubeVerts] = mix(na, nb, vec3<f32>(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

fn interpY(index : u32, i : vec3<u32>, va : f32, vb : f32) {
  let mu = ((volume.threshold - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + vec3<f32>(0.0, (volume.step.y * mu), 0.0));
  let na = normalAt(i);
  let nb = normalAt((i + vec3<u32>(0u, 1u, 0u)));
  normals[cubeVerts] = mix(na, nb, vec3<f32>(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

fn interpZ(index : u32, i : vec3<u32>, va : f32, vb : f32) {
  let mu = ((volume.threshold - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + vec3<f32>(0.0, 0.0, (volume.step.z * mu)));
  let na = normalAt(i);
  let nb = normalAt((i + vec3<u32>(0u, 0u, 1u)));
  normals[cubeVerts] = mix(na, nb, vec3<f32>(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

@compute @workgroup_size(4, 4, 4)
fn computeMain(@builtin(global_invocation_id) global_id : vec3<u32>) {
  let i0 = global_id;
  let i1 = (global_id + vec3<u32>(1u, 0u, 0u));
  let i2 = (global_id + vec3<u32>(1u, 1u, 0u));
  let i3 = (global_id + vec3<u32>(0u, 1u, 0u));
  let i4 = (global_id + vec3<u32>(0u, 0u, 1u));
  let i5 = (global_id + vec3<u32>(1u, 0u, 1u));
  let i6 = (global_id + vec3<u32>(1u, 1u, 1u));
  let i7 = (global_id + vec3<u32>(0u, 1u, 1u));
  let v0 = valueAt(i0);
  let v1 = valueAt(i1);
  let v2 = valueAt(i2);
  let v3 = valueAt(i3);
  let v4 = valueAt(i4);
  let v5 = valueAt(i5);
  let v6 = valueAt(i6);
  let v7 = valueAt(i7);
  var cubeIndex = 0u;
  if ((v0 < volume.threshold)) {
    cubeIndex = (cubeIndex | 1u);
  }
  if ((v1 < volume.threshold)) {
    cubeIndex = (cubeIndex | 2u);
  }
  if ((v2 < volume.threshold)) {
    cubeIndex = (cubeIndex | 4u);
  }
  if ((v3 < volume.threshold)) {
    cubeIndex = (cubeIndex | 8u);
  }
  if ((v4 < volume.threshold)) {
    cubeIndex = (cubeIndex | 16u);
  }
  if ((v5 < volume.threshold)) {
    cubeIndex = (cubeIndex | 32u);
  }
  if ((v6 < volume.threshold)) {
    cubeIndex = (cubeIndex | 64u);
  }
  if ((v7 < volume.threshold)) {
    cubeIndex = (cubeIndex | 128u);
  }
  let edges = tables.edges[cubeIndex];
  if (((edges & 1u) != 0u)) {
    interpX(0u, i0, v0, v1);
  }
  if (((edges & 2u) != 0u)) {
    interpY(1u, i1, v1, v2);
  }
  if (((edges & 4u) != 0u)) {
    interpX(2u, i3, v3, v2);
  }
  if (((edges & 8u) != 0u)) {
    interpY(3u, i0, v0, v3);
  }
  if (((edges & 16u) != 0u)) {
    interpX(4u, i4, v4, v5);
  }
  if (((edges & 32u) != 0u)) {
    interpY(5u, i5, v5, v6);
  }
  if (((edges & 64u) != 0u)) {
    interpX(6u, i7, v7, v6);
  }
  if (((edges & 128u) != 0u)) {
    interpY(7u, i4, v4, v7);
  }
  if (((edges & 256u) != 0u)) {
    interpZ(8u, i0, v0, v4);
  }
  if (((edges & 512u) != 0u)) {
    interpZ(9u, i1, v1, v5);
  }
  if (((edges & 1024u) != 0u)) {
    interpZ(10u, i2, v2, v6);
  }
  if (((edges & 2048u) != 0u)) {
    interpZ(11u, i3, v3, v7);
  }
  let triTableOffset = ((cubeIndex << 4u) + 1u);
  let indexCount = u32(tables.tris[(triTableOffset - 1u)]);
  var firstVertex = atomicAdd(&(drawOut.vertexCount), cubeVerts);
  let bufferOffset = ((global_id.x + (global_id.y * volume.size.x)) + ((global_id.z * volume.size.x) * volume.size.y));
  let firstIndex = (bufferOffset * 15u);
  for(var i = 0u; (i < cubeVerts); i = (i + 1u)) {
    positionsOut.values[((firstVertex * 3u) + (i * 3u))] = positions[i].x;
    positionsOut.values[(((firstVertex * 3u) + (i * 3u)) + 1u)] = positions[i].y;
    positionsOut.values[(((firstVertex * 3u) + (i * 3u)) + 2u)] = positions[i].z;
    normalsOut.values[((firstVertex * 3u) + (i * 3u))] = normals[i].x;
    normalsOut.values[(((firstVertex * 3u) + (i * 3u)) + 1u)] = normals[i].y;
    normalsOut.values[(((firstVertex * 3u) + (i * 3u)) + 2u)] = normals[i].z;
  }
  for(var i = 0u; (i < indexCount); i = (i + 1u)) {
    let index = tables.tris[(triTableOffset + i)];
    indicesOut.tris[(firstIndex + i)] = (firstVertex + indices[index]);
  }
  for(var i = indexCount; (i < 15u); i = (i + 1u)) {
    indicesOut.tris[(firstIndex + i)] = firstVertex;
  }
}
