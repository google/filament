@compute @workgroup_size(1)
fn f() {
    var a = vec3<f32>(1., 2., 3.);
    var b = 0.;
    let r : vec3<f32> = a / (b + b);
}
