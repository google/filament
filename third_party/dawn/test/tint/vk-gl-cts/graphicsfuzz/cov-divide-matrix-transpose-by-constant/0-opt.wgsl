var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m : mat2x2<f32>;
  m = (transpose(mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0))) * (1.0 / 2.0));
  let x_33 : mat2x2<f32> = m;
  if ((all((x_33[0u] == mat2x2<f32>(vec2<f32>(0.5, 1.5), vec2<f32>(1.0, 2.0))[0u])) & all((x_33[1u] == mat2x2<f32>(vec2<f32>(0.5, 1.5), vec2<f32>(1.0, 2.0))[1u])))) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
