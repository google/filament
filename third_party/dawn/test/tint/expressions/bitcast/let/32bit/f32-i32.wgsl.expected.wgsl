@compute @workgroup_size(1)
fn f() {
  let a : f32 = f32(2.003662109375f);
  let b : i32 = bitcast<i32>(a);
}
