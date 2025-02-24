var<private> p : f32;

var<workgroup> w : f32;

@group(0) @binding(1) var<storage> uniforms : vec2f;

@group(0) @binding(0) var<storage, read_write> storages : array<f32>;

fn no_uses() {
}

fn zoo() {
  p = (p * 2.0);
}

fn bar(a : f32, b : f32) {
  p = a;
  w = b;
  storages[0] = uniforms.x;
  zoo();
}

fn foo(a : f32) {
  let b : f32 = 2.0;
  bar(a, b);
  no_uses();
}

@compute @workgroup_size(1)
fn main() {
  foo(1.0);
}
