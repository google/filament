fn main() -> f32 {
  return (((2.0 * 3.0) - 4.0) / 5.0);
}

@compute @workgroup_size(2)
fn ep() {
  let a = main();
}
