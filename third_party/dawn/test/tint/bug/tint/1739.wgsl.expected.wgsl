@group(0) @binding(0) var t : texture_external;

@group(0) @binding(1) var outImage : texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(1)
fn main() {
  var red : vec4<f32> = textureLoad(t, vec2<i32>(10, 10));
  textureStore(outImage, vec2<i32>(0, 0), red);
  var green : vec4<f32> = textureLoad(t, vec2<i32>(70, 118));
  textureStore(outImage, vec2<i32>(1, 0), green);
  return;
}
