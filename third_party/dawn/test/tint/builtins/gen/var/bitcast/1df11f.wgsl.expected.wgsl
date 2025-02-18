enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn bitcast_1df11f() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = bitcast<vec2<f16>>(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_1df11f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_1df11f();
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
  out.prevent_dce = bitcast_1df11f();
  return out;
}
