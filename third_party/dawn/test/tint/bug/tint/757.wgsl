
 struct Constants {
  level : i32,
};

@group(0) @binding(0) var<uniform> constants : Constants;
@group(0) @binding(1) var myTexture : texture_2d_array<f32>;

 struct Result {
  values : array<f32>,
};
@group(0) @binding(3) var<storage, read_write> result : Result;

@compute @workgroup_size(1)
fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  var flatIndex : u32 =
    2u * 2u * GlobalInvocationID.z +
    2u * GlobalInvocationID.y +
    GlobalInvocationID.x;
  flatIndex = flatIndex * 1u;
  var texel : vec4<f32> = textureLoad(myTexture, vec2<i32>(GlobalInvocationID.xy), 0, 0);

  for (var i : u32 = 0u; i < 1u; i = i + 1u) {
    result.values[flatIndex + i] = texel.r;
  }
}
