enable f16;

@compute @workgroup_size(1)
fn f() {
  let a : vec4<f16> = vec4<f16>(1.0h, 2.0h, 3.0h, -(4.0h));
  let b : vec2<i32> = bitcast<vec2<i32>>(a);
}
