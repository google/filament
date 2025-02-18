const x_10 = vec3f(1.0f, 2.0f, 3.0f);

fn main_1() {
  let x_11 = x_10.y;
  let x_13 = x_10.xz;
  let x_14 = x_10.xzy;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
