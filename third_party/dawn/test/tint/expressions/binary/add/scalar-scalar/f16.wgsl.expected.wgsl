enable f16;

@compute @workgroup_size(1)
fn f() {
  let a = 1.0h;
  let b = 2.0h;
  let r : f16 = (a + b);
}
