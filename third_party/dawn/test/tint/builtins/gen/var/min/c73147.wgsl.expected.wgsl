@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn min_c73147() -> i32 {
  var arg_0 = 1i;
  var arg_1 = 1i;
  var res : i32 = min(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = min_c73147();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = min_c73147();
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
  out.prevent_dce = min_c73147();
  return out;
}
