enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn floor_b6e09c() -> f16 {
  var arg_0 = 1.5h;
  var res : f16 = floor(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = floor_b6e09c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = floor_b6e09c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = floor_b6e09c();
  return out;
}
