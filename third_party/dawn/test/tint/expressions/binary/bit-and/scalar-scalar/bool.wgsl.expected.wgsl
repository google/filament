@compute @workgroup_size(1)
fn f() {
  let a = true;
  let b = false;
  let r : bool = (a & b);
}
