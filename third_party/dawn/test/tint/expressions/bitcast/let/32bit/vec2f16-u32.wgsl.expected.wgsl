enable f16;

@compute @workgroup_size(1)
fn f() {
  let a : vec2<f16> = vec2<f16>(1.0h, 2.0h);
  let b : u32 = bitcast<u32>(a);
}
