var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : u32;
  a = 4u;
  let x_5 : u32 = a;
  switch((x_5 / 2u)) {
    case 2u: {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    }
    default: {
      x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
    }
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
