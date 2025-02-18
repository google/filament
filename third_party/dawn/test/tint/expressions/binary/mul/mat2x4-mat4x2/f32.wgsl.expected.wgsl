@compute @workgroup_size(1)
fn f() {
  let a = mat2x4<f32>(vec4<f32>(1.0, 2.0, 3.0, 4.0), vec4<f32>(5.0, 6.0, 7.0, 8.0));
  let b = mat4x2<f32>(vec2<f32>(-(1.0), -(2.0)), vec2<f32>(-(3.0), -(4.0)), vec2<f32>(-(5.0), -(6.0)), vec2<f32>(-(7.0), -(8.0)));
  let r : mat4x4<f32> = (a * b);
}
