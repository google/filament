@compute @workgroup_size(1)
fn f() {
    let a = vec3<f32>(1., 2., 3.);
    let b = vec3<f32>(0., 5., 0.);
    let r : vec3<f32> = a % b;
}
