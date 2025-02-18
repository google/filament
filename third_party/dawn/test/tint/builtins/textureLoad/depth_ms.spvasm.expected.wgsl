@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

var<private> tint_symbol_1 = vec4f();

fn textureLoad_6273b1() {
  var res = 0.0f;
  res = vec4f(textureLoad(arg_0, vec2i(), 1i), 0.0f, 0.0f, 0.0f).x;
  return;
}

fn tint_symbol_2(tint_symbol : vec4f) {
  tint_symbol_1 = tint_symbol;
  return;
}

fn vertex_main_1() {
  textureLoad_6273b1();
  tint_symbol_2(vec4f());
  return;
}

struct vertex_main_out {
  @builtin(position)
  tint_symbol_1_1 : vec4f,
}

@vertex
fn vertex_main() -> vertex_main_out {
  vertex_main_1();
  return vertex_main_out(tint_symbol_1);
}

fn fragment_main_1() {
  textureLoad_6273b1();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  textureLoad_6273b1();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
