var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_21_phi : bool;
  x_21_phi = false;
  loop {
    var x_25 : bool;
    var x_25_phi : bool;
    var x_30_phi : bool;
    let x_21 : bool = x_21_phi;
    x_25_phi = x_21;
    loop {
      x_25 = x_25_phi;
      x_30_phi = x_25;
      if ((1 < 0)) {
      } else {
        break;
      }
      x_30_phi = true;
      break;

      continuing {
        x_25_phi = false;
      }
    }
    let x_30 : bool = x_30_phi;
    if (x_30) {
      break;
    }
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    break;

    continuing {
      x_21_phi = false;
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
