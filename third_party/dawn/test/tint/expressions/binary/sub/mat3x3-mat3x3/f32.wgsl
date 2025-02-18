@compute @workgroup_size(1)
fn f() {
    let a = mat3x3<f32>(vec3<f32>( 1.,  2.,  3.), vec3<f32>( 4.,  5.,  6.), vec3<f32>( 7.,  8.,  9.));
    let b = mat3x3<f32>(vec3<f32>(-1., -2., -3.), vec3<f32>(-4., -5., -6.), vec3<f32>(-7., -8., -9.));
    let r : mat3x3<f32> = a - b;
}
