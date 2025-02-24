struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn returnRed_() -> vec4<f32> {
  var x_33 : bool = false;
  var x_34 : vec4<f32>;
  var x_48 : vec4<f32>;
  var x_36_phi : bool;
  var x_51_phi : vec4<f32>;
  x_36_phi = false;
  loop {
    var x_48_phi : vec4<f32>;
    var x_49_phi : bool;
    let x_36 : bool = x_36_phi;
    loop {
      let x_44 : i32 = x_6.zero;
      if ((x_44 == 1)) {
        x_33 = true;
        x_34 = vec4<f32>(1.0, 0.0, 0.0, 1.0);
        x_48_phi = vec4<f32>(1.0, 0.0, 0.0, 1.0);
        x_49_phi = true;
        break;
      }

      continuing {
        x_48_phi = vec4<f32>();
        x_49_phi = false;
        break if !(false);
      }
    }
    x_48 = x_48_phi;
    let x_49 : bool = x_49_phi;
    x_51_phi = x_48;
    if (x_49) {
      break;
    }
    x_33 = true;
    x_34 = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    x_51_phi = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    break;

    continuing {
      x_36_phi = false;
    }
  }
  let x_51 : vec4<f32> = x_51_phi;
  return x_51;
}

fn main_1() {
  loop {
    let x_30 : vec4<f32> = returnRed_();
    x_GLF_color = x_30;
    if (false) {
    } else {
      break;
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
