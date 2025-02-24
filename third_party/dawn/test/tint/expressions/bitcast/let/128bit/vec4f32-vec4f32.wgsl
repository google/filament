@compute @workgroup_size(1)
fn f() {
    let a : vec4<f32> = vec4<f32>(2.003662109375f, -513.03125f, -1024.25f, 0.0016968456f);
    let b : vec4<f32> = bitcast<vec4<f32>>(a);
}
