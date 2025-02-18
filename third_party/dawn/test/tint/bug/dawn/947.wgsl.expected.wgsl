struct Uniforms {
  u_scale : vec2<f32>,
  u_offset : vec2<f32>,
}

@binding(0) @group(0) var<uniform> uniforms : Uniforms;

struct VertexOutputs {
  @location(0)
  texcoords : vec2<f32>,
  @builtin(position)
  position : vec4<f32>,
}

@vertex
fn vs_main(@builtin(vertex_index) VertexIndex : u32) -> VertexOutputs {
  var texcoord = array<vec2<f32>, 3>(vec2<f32>(-(0.5), 0.0), vec2<f32>(1.5, 0.0), vec2<f32>(0.5, 2.0));
  var output : VertexOutputs;
  output.position = vec4<f32>(((texcoord[VertexIndex] * 2.0) - vec2<f32>(1.0, 1.0)), 0.0, 1.0);
  var flipY = (uniforms.u_scale.y < 0.0);
  if (flipY) {
    output.texcoords = ((((texcoord[VertexIndex] * uniforms.u_scale) + uniforms.u_offset) * vec2<f32>(1.0, -(1.0))) + vec2<f32>(0.0, 1.0));
  } else {
    output.texcoords = ((((texcoord[VertexIndex] * vec2<f32>(1.0, -(1.0))) + vec2<f32>(0.0, 1.0)) * uniforms.u_scale) + uniforms.u_offset);
  }
  return output;
}

@binding(1) @group(0) var mySampler : sampler;

@binding(2) @group(0) var myTexture : texture_2d<f32>;

@fragment
fn fs_main(@location(0) texcoord : vec2<f32>) -> @location(0) vec4<f32> {
  var clampedTexcoord = clamp(texcoord, vec2<f32>(0.0, 0.0), vec2<f32>(1.0, 1.0));
  if (!(all((clampedTexcoord == texcoord)))) {
    discard;
  }
  var srcColor = vec4<f32>(0);
  return srcColor;
}
