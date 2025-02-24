@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn bitcast_ca5c3f() -> f32 {
  var res : f32 = bitcast<f32>(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_ca5c3f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_ca5c3f();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = bitcast_ca5c3f();
  return out;
}
