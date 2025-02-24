struct Uniforms {
  dstTextureFlipY : u32,
  isFloat16 : u32,
  isRGB10A2Unorm : u32,
  channelCount : u32,
}

struct OutputBuf {
  result : array<u32>,
}

@group(0) @binding(0) var src : texture_2d<f32>;

@group(0) @binding(1) var dst : texture_2d<f32>;

@group(0) @binding(2) var<storage, read_write> output : OutputBuf;

@group(0) @binding(3) var<uniform> uniforms : Uniforms;

fn ConvertToFp16FloatValue(fp32 : f32) -> u32 {
  return 1u;
}

@compute @workgroup_size(1, 1, 1)
fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
  var size = textureDimensions(src);
  var dstTexCoord = GlobalInvocationID.xy;
  var srcTexCoord = dstTexCoord;
  if ((uniforms.dstTextureFlipY == 1u)) {
    srcTexCoord.y = ((size.y - dstTexCoord.y) - 1);
  }
  var srcColor : vec4<f32> = textureLoad(src, srcTexCoord, 0);
  var dstColor : vec4<f32> = textureLoad(dst, dstTexCoord, 0);
  var success : bool = true;
  var srcColorBits : vec4<u32>;
  var dstColorBits : vec4<u32> = vec4<u32>(dstColor);
  for(var i : u32 = 0u; (i < uniforms.channelCount); i = (i + 1u)) {
    srcColorBits[i] = ConvertToFp16FloatValue(srcColor[i]);
    success = (success && (srcColorBits[i] == dstColorBits[i]));
  }
  var outputIndex : u32 = ((GlobalInvocationID.y * u32(size.x)) + GlobalInvocationID.x);
  if (success) {
    output.result[outputIndex] = u32(1);
  } else {
    output.result[outputIndex] = u32(0);
  }
}
