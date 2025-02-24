@compute @workgroup_size(1)
fn f() {
    let a : vec3<f32> = vec3<f32>(2.003662109375f, -513.03125f, -1024.25f);
    let b : vec3<f32> = bitcast<vec3<f32>>(a);
}
