struct Out {
  @builtin(position) @invariant pos : vec4<f32>,
};

@vertex
fn main() -> Out {
  return Out();
}
