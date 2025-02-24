var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var GLF_live9r : i32;
  var g : f32;
  loop {
    if (true) {
    } else {
      break;
    }
    if (true) {
      break;
    }
    let x_31 : i32 = GLF_live9r;
    let x_32 : i32 = clamp(x_31, 0, 1);
  }
  g = 3.0;
  let x_33 : f32 = g;
  x_GLF_color = vec4<f32>(smoothstep(0.0, 1.0, x_33), 0.0, 0.0, 1.0);
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
