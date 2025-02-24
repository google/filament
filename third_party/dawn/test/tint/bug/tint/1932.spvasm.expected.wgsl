const x_22 = vec2f(2.0f);

fn main_1() {
  let x_10 = distance(x_22, x_22);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
