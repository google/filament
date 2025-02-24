// flags:  --hlsl-shader-model 62
enable f16;

struct Interface {
  @location(1) col1 : f32,
  @location(2) col2 : f16,
  @builtin(position) pos : vec4<f32>,
};

@vertex
fn vert_main() -> Interface {
  return Interface(0.4, 0.6h, vec4<f32>());
}

@fragment
fn frag_main(colors : Interface) {
  let r : f32 = colors.col1;
  let g : f16 = colors.col2;
}
