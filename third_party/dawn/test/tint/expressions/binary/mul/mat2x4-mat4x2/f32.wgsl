@compute @workgroup_size(1)
fn f() {
    let a = mat2x4<f32>(vec4<f32>(1., 2., 3., 4.), vec4<f32>(5., 6., 7., 8.));
    let b = mat4x2<f32>(vec2<f32>(-1., -2.), vec2<f32>(-3., -4.), vec2<f32>(-5., -6.), vec2<f32>(-7., -8.));
    let r : mat4x4<f32> = a * b;
}
