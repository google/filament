@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn select_fb7e53() -> i32 {
  var arg_0 = vec2<bool>(true);
  var arg_1 = vec2<bool>(true);
  var arg_2 = true;
  var res : vec2<bool> = select(arg_0, arg_1, arg_2);
  return select(0, 1, all((res == vec2<bool>())));
}

@fragment
fn fragment_main() {
  prevent_dce = select_fb7e53();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_fb7e53();
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
  out.prevent_dce = select_fb7e53();
  return out;
}
