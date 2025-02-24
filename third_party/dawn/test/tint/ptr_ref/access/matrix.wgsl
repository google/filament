@compute @workgroup_size(1)
fn main() {
  var m : mat3x3<f32> = mat3x3<f32>(vec3<f32>(1., 2., 3.), vec3<f32>(4., 5., 6.), vec3<f32>(7., 8., 9.));
  let v : ptr<function, vec3<f32>> = &m[1];
  *v = vec3<f32>(5., 5., 5.);
}
