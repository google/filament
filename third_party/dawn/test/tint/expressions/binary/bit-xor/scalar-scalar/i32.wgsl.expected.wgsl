@compute @workgroup_size(1)
fn f() {
  let a = 1;
  let b = 2;
  let r : i32 = (a ^ b);
}
