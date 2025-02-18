@compute @workgroup_size(1)
fn f() {
    const a : vec2<f32> = vec2<f32>(2.003662109375f, -513.03125f);
    let b : vec2<f32> = bitcast<vec2<f32>>(a);
}
