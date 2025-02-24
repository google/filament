@compute @workgroup_size(1)
fn f() {
  const a : vec3<u32> = vec3<u32>(1073757184u, 3288351232u, 3296724992u);
  let b : vec3<i32> = bitcast<vec3<i32>>(a);
}
