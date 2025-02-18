@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn dot_fc5f7c() -> i32 {
  var arg_0 = vec2<i32>(1i);
  var arg_1 = vec2<i32>(1i);
  var res : i32 = dot(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dot_fc5f7c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = dot_fc5f7c();
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
  out.prevent_dce = dot_fc5f7c();
  return out;
}
