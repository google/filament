@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn sign_3233fa() -> i32 {
  var arg_0 = 1i;
  var res : i32 = sign(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = sign_3233fa();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = sign_3233fa();
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
  out.prevent_dce = sign_3233fa();
  return out;
}
