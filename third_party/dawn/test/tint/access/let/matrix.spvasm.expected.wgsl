fn main_1() {
  let x_24 = mat3x3f(vec3f(1.0f, 2.0f, 3.0f), vec3f(4.0f, 5.0f, 6.0f), vec3f(7.0f, 8.0f, 9.0f))[1u].y;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
