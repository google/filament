struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_36 : bool = false;
  var x_37 : bool;
  var x_7 : i32;
  var x_38 : bool;
  var color : vec3<f32>;
  var x_40 : bool;
  var x_43 : vec3<f32>;
  var x_40_phi : bool;
  var x_42_phi : vec3<f32>;
  var x_56_phi : bool;
  var x_58_phi : bool;
  x_40_phi = false;
  x_42_phi = vec3<f32>();
  loop {
    var x_43_phi : vec3<f32>;
    x_40 = x_40_phi;
    let x_42 : vec3<f32> = x_42_phi;
    let x_47 : f32 = x_5.injectionSwitch.y;
    x_43_phi = x_42;
    if ((x_47 < 0.0)) {
      color = vec3<f32>(1.0, 1.0, 1.0);
      x_43_phi = vec3<f32>(1.0, 1.0, 1.0);
    }
    x_43 = x_43_phi;

    continuing {
      x_40_phi = x_40;
      x_42_phi = x_43;
      break if !(false);
    }
  }
  x_36 = false;
  x_56_phi = x_40;
  x_58_phi = false;
  loop {
    var x_62 : bool;
    var x_62_phi : bool;
    var x_64_phi : bool;
    var x_65_phi : i32;
    var x_70_phi : bool;
    var x_71_phi : bool;
    let x_56 : bool = x_56_phi;
    let x_58 : bool = x_58_phi;
    x_7 = 0;
    x_62_phi = x_56;
    x_64_phi = false;
    x_65_phi = 0;
    loop {
      x_62 = x_62_phi;
      let x_64 : bool = x_64_phi;
      let x_65 : i32 = x_65_phi;
      let x_68 : bool = (0 < 1);
      x_70_phi = x_62;
      x_71_phi = false;
      if (true) {
      } else {
        break;
      }
      x_36 = true;
      x_37 = true;
      x_70_phi = true;
      x_71_phi = true;
      break;

      continuing {
        x_62_phi = false;
        x_64_phi = false;
        x_65_phi = 0;
      }
    }
    let x_70 : bool = x_70_phi;
    let x_71 : bool = x_71_phi;
    if (true) {
      break;
    }
    x_36 = true;
    break;

    continuing {
      x_56_phi = false;
      x_58_phi = false;
    }
  }
  x_38 = true;
  let x_73 : f32 = select(0.0, 1.0, true);
  x_GLF_color = (vec4<f32>(x_43.x, x_43.y, x_43.z, 1.0) + vec4<f32>(x_73, x_73, x_73, x_73));
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
