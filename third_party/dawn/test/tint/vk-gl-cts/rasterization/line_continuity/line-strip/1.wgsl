var<private> color_out : vec4<f32>;

@group(0) @binding(0) var texture : texture_2d<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  let x_19 : vec4<f32> = gl_FragCoord;
  let x_22 : vec4<f32> = textureLoad(texture, vec2<i32>(vec2<f32>(x_19.x, x_19.y)), 0);
  color_out = x_22;
  return;
}

struct main_out {
  @location(0)
  color_out_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(color_out);
}
