enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn normalize_b8cb8d() -> vec4<f16> {
  var arg_0 = vec4<f16>(1.0h);
  var res : vec4<f16> = normalize(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = normalize_b8cb8d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = normalize_b8cb8d();
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
  out.prevent_dce = normalize_b8cb8d();
  return out;
}
