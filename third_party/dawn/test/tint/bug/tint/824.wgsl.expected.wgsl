struct Output {
  @builtin(position)
  Position : vec4<f32>,
  @location(0)
  color : vec4<f32>,
}

@vertex
fn main(@builtin(vertex_index) VertexIndex : u32, @builtin(instance_index) InstanceIndex : u32) -> Output {
  let zv : array<vec2<f32>, 4> = array<vec2<f32>, 4>(vec2<f32>(0.2000000000000000111, 0.2000000000000000111), vec2<f32>(0.2999999999999999889, 0.2999999999999999889), vec2<f32>(-(0.10000000000000000555), -(0.10000000000000000555)), vec2<f32>(1.10000000000000008882, 1.10000000000000008882));
  let z : f32 = zv[InstanceIndex].x;
  var output : Output;
  output.Position = vec4<f32>(0.5, 0.5, z, 1.0);
  let colors : array<vec4<f32>, 4> = array<vec4<f32>, 4>(vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  output.color = colors[InstanceIndex];
  return output;
}
