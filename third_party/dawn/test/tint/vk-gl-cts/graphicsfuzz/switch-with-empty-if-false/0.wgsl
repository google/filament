var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  switch(0) {
    case 0: {
      if (false) {
      }
    }
    default: {
    }
  }
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
