@compute @workgroup_size(1)
fn f() {
    let a : vec4<i32> = vec4<i32>(1073757184i, -1006616064i, -998242304i, 987654321i);
    let b : vec4<f32> = bitcast<vec4<f32>>(a);
}
