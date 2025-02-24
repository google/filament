enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn min_ac84d6() -> f16 {
  var arg_0 = 1.0h;
  var arg_1 = 1.0h;
  var res : f16 = min(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = min_ac84d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = min_ac84d6();
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
  out.prevent_dce = min_ac84d6();
  return out;
}
