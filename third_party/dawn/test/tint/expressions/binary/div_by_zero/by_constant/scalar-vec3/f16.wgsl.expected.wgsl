enable f16;

@compute @workgroup_size(1)
fn f() {
  let a = 4.0h;
  let b = vec3<f16>(0.0h, 2.0h, 0.0h);
  let r : vec3<f16> = (a / b);
}
