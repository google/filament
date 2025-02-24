enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn sinh_924f19() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = sinh(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = sinh_924f19();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = sinh_924f19();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<f16>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = sinh_924f19();
  return out;
}
