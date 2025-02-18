struct block0 {
  data : vec4<f32>,
}

@group(0) @binding(1) var<storage, read_write> x_4 : block0;

fn main_1() {
  x_4.data = vec4<f32>(1.0, 2.0, 3.0, 4.0);
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main() {
  main_1();
}
