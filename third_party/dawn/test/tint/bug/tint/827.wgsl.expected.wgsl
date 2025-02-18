struct Result {
  values : array<f32>,
}

const width : u32 = 128u;

@group(0) @binding(0) var tex : texture_depth_2d;

@group(0) @binding(1) var<storage, read_write> result : Result;

@compute @workgroup_size(1)
fn main(@builtin(global_invocation_id) GlobalInvocationId : vec3<u32>) {
  result.values[((GlobalInvocationId.y * width) + GlobalInvocationId.x)] = textureLoad(tex, vec2<i32>(i32(GlobalInvocationId.x), i32(GlobalInvocationId.y)), 0);
}
