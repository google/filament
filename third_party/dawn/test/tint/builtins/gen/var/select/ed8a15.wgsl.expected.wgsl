@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn select_ed8a15() -> i32 {
  var arg_0 = 1i;
  var arg_1 = 1i;
  var arg_2 = true;
  var res : i32 = select(arg_0, arg_1, arg_2);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = select_ed8a15();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_ed8a15();
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
  out.prevent_dce = select_ed8a15();
  return out;
}
