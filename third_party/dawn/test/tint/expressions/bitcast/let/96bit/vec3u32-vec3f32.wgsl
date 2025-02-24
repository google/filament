@compute @workgroup_size(1)
fn f() {
    let a : vec3<u32> = vec3<u32>(1073757184u, 3288351232u, 3296724992u);
    let b : vec3<f32> = bitcast<vec3<f32>>(a);
}
