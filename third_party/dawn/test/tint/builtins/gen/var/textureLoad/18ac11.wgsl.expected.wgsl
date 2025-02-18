@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32sint, read>;

fn textureLoad_18ac11() -> vec4<i32> {
  var arg_1 = 1u;
  var res : vec4<i32> = textureLoad(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_18ac11();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_18ac11();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureLoad_18ac11();
  return out;
}
