fn x_200(x_201 : ptr<function, vec2f>) -> f32 {
  let x_212 = (*(x_201)).x;
  return x_212;
}

fn main_1() {
  var x_11 : vec2f;
  let x_12 = x_200(&(x_11));
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
