@compute @workgroup_size(1)
fn f() {
    const a : i32 = i32(1073757184i);
    let b : f32 = bitcast<f32>(a);
}
