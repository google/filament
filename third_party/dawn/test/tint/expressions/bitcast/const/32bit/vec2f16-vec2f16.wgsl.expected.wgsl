enable f16;

@compute @workgroup_size(1)
fn f() {
  const a : vec2<f16> = vec2<f16>(1.0h, 2.0h);
  let b : vec2<f16> = bitcast<vec2<f16>>(a);
}
