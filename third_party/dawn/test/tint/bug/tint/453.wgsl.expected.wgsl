@group(0) @binding(0) var Src : texture_2d<u32>;

@group(0) @binding(1) var Dst : texture_storage_2d<r32uint, write>;

@compute @workgroup_size(1)
fn main() {
  var srcValue : vec4<u32>;
  let x_22 : vec4<u32> = textureLoad(Src, vec2<i32>(0, 0), 0);
  srcValue = x_22;
  let x_24 : u32 = srcValue.x;
  let x_25 : u32 = (x_24 + 1u);
  let x_27 : vec4<u32> = srcValue;
  textureStore(Dst, vec2<i32>(0, 0), x_27.xxxx);
  return;
}
