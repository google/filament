@compute @workgroup_size(1)
fn f() {
    let a : f32 = f32(2.003662109375f);
    let b : f32 = bitcast<f32>(a);
}
