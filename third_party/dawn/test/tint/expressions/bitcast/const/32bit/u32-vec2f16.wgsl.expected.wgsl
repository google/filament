enable f16;

@compute @workgroup_size(1)
fn f() {
  const a : u32 = u32(1073757184u);
  let b : vec2<f16> = bitcast<vec2<f16>>(a);
}
