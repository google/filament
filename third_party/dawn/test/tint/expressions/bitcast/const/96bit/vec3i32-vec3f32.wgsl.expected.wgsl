@compute @workgroup_size(1)
fn f() {
  const a : vec3<i32> = vec3<i32>(1073757184i, -(1006616064i), -(998242304i));
  let b : vec3<f32> = bitcast<vec3<f32>>(a);
}
