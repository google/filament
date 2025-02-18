enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn trunc_ce7c17() -> vec4<f16> {
  var arg_0 = vec4<f16>(1.5h);
  var res : vec4<f16> = trunc(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = trunc_ce7c17();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = trunc_ce7c17();
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
  out.prevent_dce = trunc_ce7c17();
  return out;
}
