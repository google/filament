enable f16;

@compute @workgroup_size(1)
fn f() {
  let a : f16 = f16(1.0h);
  let b : f16 = bitcast<f16>(a);
}
