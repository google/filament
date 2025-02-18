@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn bitcast_e61c57() -> i32 {
  var arg_0 = 1u;
  var res : i32 = bitcast<i32>(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_e61c57();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_e61c57();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : i32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = bitcast_e61c57();
  return out;
}
