@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg32uint, read>;

fn textureLoad_c8ed19() -> vec4<u32> {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = 1u;
  var res : vec4<u32> = textureLoad(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_c8ed19();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_c8ed19();
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
  out.prevent_dce = textureLoad_c8ed19();
  return out;
}
