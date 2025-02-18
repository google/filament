enable chromium_internal_graphite;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r8unorm, read>;

fn textureLoad_35d464() -> vec4<f32> {
  var arg_1 = vec2<i32>(1i);
  var arg_2 = 1i;
  var res : vec4<f32> = textureLoad(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_35d464();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_35d464();
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
  out.prevent_dce = textureLoad_35d464();
  return out;
}
