@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_storage_1d<bgra8unorm, read>;

fn textureLoad_b7f74f() -> vec4<f32> {
  var res : vec4<f32> = textureLoad(arg_0, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_b7f74f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_b7f74f();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureLoad_b7f74f();
  return out;
}
