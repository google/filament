@vertex
fn main(
  @builtin(vertex_index) vertex_index : u32,
  @builtin(instance_index) instance_index : u32,
) -> @builtin(position) vec4<f32> {
  let foo : u32 = vertex_index + instance_index;
  return vec4<f32>();
}
