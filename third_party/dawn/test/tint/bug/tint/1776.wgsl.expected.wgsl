struct S {
  a : vec4<f32>,
  b : i32,
}

@group(0) @binding(0) var<storage> sb : array<S>;

@compute @workgroup_size(1)
fn main() {
  let x = sb[1];
}
