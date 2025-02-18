var<private> frag_color : vec4<f32>;

var<private> color_out : vec4<f32>;

fn main_1() {
  var iv : vec2<i32>;
  let x_28 : vec4<f32> = frag_color;
  iv = vec2<i32>((vec2<f32>(x_28.x, x_28.y) * 256.0));
  let x_33 : i32 = iv.y;
  if ((((x_33 / 2) & 64) == 64)) {
    color_out = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    color_out = vec4<f32>(0.0, 1.0, 1.0, 1.0);
  }
  return;
}

struct main_out {
  @location(0)
  color_out_1 : vec4<f32>,
}

@fragment
fn main(@location(1) frag_color_param : vec4<f32>) -> main_out {
  frag_color = frag_color_param;
  main_1();
  return main_out(color_out);
}
