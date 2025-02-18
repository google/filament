@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

var<private> tint_symbol_1 = vec4f();

fn textureDimensions_f60bdb() {
  var res = vec2i();
  res = vec2i(textureDimensions(arg_0));
  return;
}

fn tint_symbol_2(tint_symbol : vec4f) {
  tint_symbol_1 = tint_symbol;
  return;
}

fn vertex_main_1() {
  textureDimensions_f60bdb();
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
  textureDimensions_f60bdb();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  textureDimensions_f60bdb();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
