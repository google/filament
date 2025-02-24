@compute @workgroup_size(1)
fn f() {
  const a : vec4<i32> = vec4<i32>(1073757184i, -(1006616064i), -(998242304i), 987654321i);
  let b : vec4<i32> = bitcast<vec4<i32>>(a);
}
