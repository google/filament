enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn bitcast_66e93d() -> vec2<f16> {
  var arg_0 = 1u;
  var res : vec2<f16> = bitcast<vec2<f16>>(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_66e93d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_66e93d();
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
  out.prevent_dce = bitcast_66e93d();
  return out;
}
