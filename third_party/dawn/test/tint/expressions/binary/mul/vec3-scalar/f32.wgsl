@compute @workgroup_size(1)
fn f() {
    let a = vec3<f32>(1., 2., 3.);
    let b = 4.;
    let r : vec3<f32> = a * b;
}
