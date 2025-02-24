var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_36 : bool = false;
  var x_37 : vec3<f32>;
  var x_6 : i32;
  var x_38 : vec3<f32>;
  var x_51 : vec3<f32>;
  var x_54 : vec3<f32>;
  var x_40_phi : bool;
  var x_55_phi : vec3<f32>;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  x_36 = false;
  x_40_phi = false;
  loop {
    var x_45 : bool;
    var x_45_phi : bool;
    var x_7_phi : i32;
    var x_51_phi : vec3<f32>;
    var x_52_phi : bool;
    let x_40 : bool = x_40_phi;
    x_6 = 0;
    x_45_phi = x_40;
    x_7_phi = 0;
    loop {
      x_45 = x_45_phi;
      let x_7 : i32 = x_7_phi;
      x_51_phi = vec3<f32>();
      x_52_phi = x_45;
      if ((x_7 < 0)) {
      } else {
        break;
      }
      x_36 = true;
      x_37 = vec3<f32>(1.0, 1.0, 1.0);
      x_51_phi = vec3<f32>(1.0, 1.0, 1.0);
      x_52_phi = true;
      break;

      continuing {
        x_45_phi = false;
        x_7_phi = 0;
      }
    }
    x_51 = x_51_phi;
    let x_52 : bool = x_52_phi;
    x_55_phi = x_51;
    if (x_52) {
      break;
    }
    x_54 = vec3<f32>();
    x_36 = true;
    x_55_phi = x_54;
    break;

    continuing {
      x_40_phi = false;
    }
  }
  let x_55 : vec3<f32> = x_55_phi;
  x_38 = x_55;
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

fn GLF_live4drawShape_() -> vec3<f32> {
  var x_57 : bool = false;
  var x_58 : vec3<f32>;
  var i : i32;
  var x_71 : vec3<f32>;
  var x_74 : vec3<f32>;
  var x_60_phi : bool;
  var x_75_phi : vec3<f32>;
  x_60_phi = false;
  loop {
    var x_65 : bool;
    var x_65_phi : bool;
    var x_8_phi : i32;
    var x_71_phi : vec3<f32>;
    var x_72_phi : bool;
    let x_60 : bool = x_60_phi;
    i = 0;
    x_65_phi = x_60;
    x_8_phi = 0;
    loop {
      x_65 = x_65_phi;
      let x_8 : i32 = x_8_phi;
      x_71_phi = vec3<f32>();
      x_72_phi = x_65;
      if ((x_8 < 0)) {
      } else {
        break;
      }
      x_57 = true;
      x_58 = vec3<f32>(1.0, 1.0, 1.0);
      x_71_phi = vec3<f32>(1.0, 1.0, 1.0);
      x_72_phi = true;
      break;

      continuing {
        x_65_phi = false;
        x_8_phi = 0;
      }
    }
    x_71 = x_71_phi;
    let x_72 : bool = x_72_phi;
    x_75_phi = x_71;
    if (x_72) {
      break;
    }
    x_74 = vec3<f32>();
    x_57 = true;
    x_75_phi = x_74;
    break;

    continuing {
      x_60_phi = false;
    }
  }
  let x_75 : vec3<f32> = x_75_phi;
  return x_75;
}
