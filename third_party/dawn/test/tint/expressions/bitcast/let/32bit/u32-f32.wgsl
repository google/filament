@compute @workgroup_size(1)
fn f() {
    let a : u32 = u32(1073757184u);
    let b : f32 = bitcast<f32>(a);
}
