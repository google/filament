@compute @workgroup_size(1)
fn f() {
    const a : vec2<i32> = vec2<i32>(1073757184i, -1006616064i);
    let b : vec2<f32> = bitcast<vec2<f32>>(a);
}
