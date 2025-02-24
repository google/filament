@compute @workgroup_size(1)
fn f0() {
  let a = 2147483647;
  let b = 1;
  let c = (a + 1);
}

fn f1() {
  let a = 1;
  let b = (-(2147483648) - a);
}
