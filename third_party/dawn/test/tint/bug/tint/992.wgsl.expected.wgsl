@fragment
fn frag_main() -> @location(0) vec4<f32> {
  var b : f32 = 0.0;
  var v : vec3<f32> = vec3<f32>(b);
  return vec4<f32>(v, 1.0);
}
