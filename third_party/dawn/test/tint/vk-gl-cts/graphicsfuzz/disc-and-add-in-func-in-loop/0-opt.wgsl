struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn x_51() {
  discard;
}

fn main_1() {
  loop {
    var x_31 : bool;
    var x_30_phi : bool;
    x_30_phi = false;
    loop {
      var x_31_phi : bool;
      let x_30 : bool = x_30_phi;
      loop {
        var x_52 : vec4<f32>;
        var x_54 : vec4<f32>;
        var x_55_phi : vec4<f32>;
        let x_36 : f32 = x_5.injectionSwitch.y;
        x_31_phi = x_30;
        if ((x_36 > 0.0)) {
        } else {
          break;
        }
        loop {
          let x_46 : f32 = x_5.injectionSwitch.x;
          if ((x_46 > 0.0)) {
            x_51();
          }
          x_54 = (vec4<f32>(1.0, 0.0, 0.0, 1.0) + vec4<f32>(x_46, x_46, x_46, x_46));
          x_55_phi = x_54;
          break;
        }
        let x_55 : vec4<f32> = x_55_phi;
        x_GLF_color = x_55;
        x_31_phi = true;
        break;
      }
      x_31 = x_31_phi;
      if (x_31) {
        break;
      } else {
        continue;
      }

      continuing {
        x_30_phi = x_31;
      }
    }
    if (x_31) {
      break;
    }
    break;
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
