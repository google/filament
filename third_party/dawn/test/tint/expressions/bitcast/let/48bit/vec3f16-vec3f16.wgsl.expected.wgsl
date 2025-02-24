enable f16;

@compute @workgroup_size(1)
fn f() {
  let a : vec3<f16> = vec3<f16>(1.0h, 2.0h, 3.0h);
  let b : vec3<f16> = bitcast<vec3<f16>>(a);
}
