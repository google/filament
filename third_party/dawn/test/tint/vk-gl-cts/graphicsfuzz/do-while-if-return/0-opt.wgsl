struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> i32 {
  var loop_count : i32;
  var x_38_phi : i32;
  loop_count = 0;
  x_38_phi = 0;
  loop {
    var x_39 : i32;
    var x_45_phi : i32;
    let x_38 : i32 = x_38_phi;
    let x_43 : i32 = (x_38 + 1);
    loop_count = x_43;
    x_45_phi = x_43;
    loop {
      let x_45 : i32 = x_45_phi;
      x_39 = (x_45 + 1);
      loop_count = x_39;
      let x_50 : f32 = x_7.injectionSwitch.x;
      let x_52 : f32 = x_7.injectionSwitch.y;
      if ((x_50 < x_52)) {
        return 1;
      }
      let x_57 : f32 = x_7.injectionSwitch.x;
      let x_59 : f32 = x_7.injectionSwitch.y;
      if ((x_57 < x_59)) {
        break;
      }

      continuing {
        x_45_phi = x_39;
        break if !(x_39 < 100);
      }
    }

    continuing {
      x_38_phi = x_39;
      break if !(x_39 < 100);
    }
  }
  return 0;
}

fn main_1() {
  let x_31 : i32 = func_();
  if ((x_31 == 1)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
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
