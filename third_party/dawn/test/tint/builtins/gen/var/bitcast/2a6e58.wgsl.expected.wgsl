enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn bitcast_2a6e58() -> vec2<f32> {
  var arg_0 = vec4<f16>(1.0h);
  var res : vec2<f32> = bitcast<vec2<f32>>(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_2a6e58();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_2a6e58();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = bitcast_2a6e58();
  return out;
}
