enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn abs_fd247f() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = abs(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = abs_fd247f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = abs_fd247f();
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
  out.prevent_dce = abs_fd247f();
  return out;
}
