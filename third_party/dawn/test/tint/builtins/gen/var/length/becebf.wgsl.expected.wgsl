@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn length_becebf() -> f32 {
  var arg_0 = vec4<f32>(0.0f);
  var res : f32 = length(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = length_becebf();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = length_becebf();
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
  out.prevent_dce = length_becebf();
  return out;
}
