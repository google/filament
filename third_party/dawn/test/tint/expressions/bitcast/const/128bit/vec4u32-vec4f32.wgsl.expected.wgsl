@compute @workgroup_size(1)
fn f() {
  const a : vec4<u32> = vec4<u32>(1073757184u, 3288351232u, 3296724992u, 987654321u);
  let b : vec4<f32> = bitcast<vec4<f32>>(a);
}
