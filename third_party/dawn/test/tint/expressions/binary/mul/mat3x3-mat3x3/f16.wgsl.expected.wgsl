enable f16;

@compute @workgroup_size(1)
fn f() {
  let a = mat3x3<f16>(vec3<f16>(1.0h, 2.0h, 3.0h), vec3<f16>(4.0h, 5.0h, 6.0h), vec3<f16>(7.0h, 8.0h, 9.0h));
  let b = mat3x3<f16>(vec3<f16>(-(1.0h), -(2.0h), -(3.0h)), vec3<f16>(-(4.0h), -(5.0h), -(6.0h)), vec3<f16>(-(7.0h), -(8.0h), -(9.0h)));
  let r : mat3x3<f16> = (a * b);
}
