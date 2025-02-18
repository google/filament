fn main_1() {
  var v = vec3f();
  let x_14 = v.y;
  let x_17 = v.xz;
  let x_19 = v.xzy;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
