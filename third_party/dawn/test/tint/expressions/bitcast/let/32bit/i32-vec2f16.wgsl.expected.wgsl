enable f16;

@compute @workgroup_size(1)
fn f() {
  let a : i32 = i32(1073757184i);
  let b : vec2<f16> = bitcast<vec2<f16>>(a);
}
