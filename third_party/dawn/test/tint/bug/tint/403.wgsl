
struct vertexUniformBuffer1 {
  transform1 : mat2x2<f32>,
};
struct vertexUniformBuffer2 {
  transform2 : mat2x2<f32>,
};

@group(0) @binding(0) var<uniform> x_20 : vertexUniformBuffer1;
@group(1) @binding(0) var<uniform> x_26 : vertexUniformBuffer2;


@vertex
fn main(@builtin(vertex_index) gl_VertexIndex : u32) -> @builtin(position) vec4<f32> {
  var indexable : array<vec2<f32>, 3>;
  let x_23 : mat2x2<f32> = x_20.transform1;
  let x_28 : mat2x2<f32> = x_26.transform2;
  let x_46 : u32 = gl_VertexIndex;
  indexable = array<vec2<f32>, 3>(vec2<f32>(-1.0, 1.0), vec2<f32>(1.0, 1.0), vec2<f32>(-1.0, -1.0));
  let x_51 : vec2<f32> = indexable[x_46];
  let x_52 : vec2<f32> = (mat2x2<f32>((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])) * x_51);
  return vec4<f32>(x_52.x, x_52.y, 0.0, 1.0);
}
