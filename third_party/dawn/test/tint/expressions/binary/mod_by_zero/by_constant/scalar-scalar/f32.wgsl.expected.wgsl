@compute @workgroup_size(1)
fn f() {
  let a = 1.0;
  let b = 0.0;
  let r : f32 = (a % b);
}
