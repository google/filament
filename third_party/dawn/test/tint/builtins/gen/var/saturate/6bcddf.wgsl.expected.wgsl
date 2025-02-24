@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn saturate_6bcddf() -> vec3<f32> {
  var arg_0 = vec3<f32>(2.0f);
  var res : vec3<f32> = saturate(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = saturate_6bcddf();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = saturate_6bcddf();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = saturate_6bcddf();
  return out;
}
