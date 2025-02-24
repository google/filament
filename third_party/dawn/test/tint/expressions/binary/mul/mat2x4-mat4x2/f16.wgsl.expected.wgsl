enable f16;

@compute @workgroup_size(1)
fn f() {
  let a = mat2x4<f16>(vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h), vec4<f16>(5.0h, 6.0h, 7.0h, 8.0h));
  let b = mat4x2<f16>(vec2<f16>(-(1.0h), -(2.0h)), vec2<f16>(-(3.0h), -(4.0h)), vec2<f16>(-(5.0h), -(6.0h)), vec2<f16>(-(7.0h), -(8.0h)));
  let r : mat4x4<f16> = (a * b);
}
