struct Uniforms {
  modelViewProjectionMatrix : mat4x4<f32>,
}

@binding(0) @group(0) var<uniform> uniforms : Uniforms;

struct VertexInput {
  @location(0)
  cur_position : vec4<f32>,
  @location(1)
  color : vec4<f32>,
}

struct VertexOutput {
  @location(0)
  vtxFragColor : vec4<f32>,
  @builtin(position)
  Position : vec4<f32>,
}

@vertex
fn vtx_main(input : VertexInput) -> VertexOutput {
  var output : VertexOutput;
  output.Position = (uniforms.modelViewProjectionMatrix * input.cur_position);
  output.vtxFragColor = input.color;
  return output;
}

@fragment
fn frag_main(@location(0) fragColor : vec4<f32>) -> @location(0) vec4<f32> {
  return fragColor;
}
