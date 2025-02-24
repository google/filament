@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@group(1) @binding(1) var arg_1 : texture_cube<u32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_3b32cc() -> vec4<u32> {
  var res : vec4<u32> = textureGather(1i, arg_1, arg_2, vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureGather_3b32cc();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureGather_3b32cc();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureGather_3b32cc();
  return out;
}
