enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn acos_203628() -> vec4<f16> {
  var res : vec4<f16> = acos(vec4<f16>(0.96875h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = acos_203628();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = acos_203628();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f16>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = acos_203628();
  return out;
}
