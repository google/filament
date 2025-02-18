struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn makeFrame_() -> f32 {
  var x_60 : f32;
  var x_63_phi : f32;
  loop {
    var x_41 : bool;
    var x_44 : f32;
    var x_45 : f32;
    var x_42 : bool;
    var x_41_phi : bool;
    var x_8_phi : i32;
    var x_44_phi : f32;
    var x_60_phi : f32;
    var x_61_phi : bool;
    x_41_phi = false;
    x_8_phi = 0;
    x_44_phi = 0.0;
    loop {
      var x_50 : f32;
      var x_9 : i32;
      var x_52 : bool;
      var x_7 : i32;
      var x_50_phi : f32;
      var x_9_phi : i32;
      var x_52_phi : bool;
      var x_45_phi : f32;
      var x_42_phi : bool;
      x_41 = x_41_phi;
      let x_8 : i32 = x_8_phi;
      x_44 = x_44_phi;
      x_60_phi = x_44;
      x_61_phi = x_41;
      if ((x_8 < 1)) {
      } else {
        break;
      }
      x_50_phi = x_44;
      x_9_phi = x_8;
      x_52_phi = x_41;
      loop {
        x_50 = x_50_phi;
        x_9 = x_9_phi;
        x_52 = x_52_phi;
        let x_54 : f32 = x_6.injectionSwitch.y;
        x_45_phi = x_50;
        x_42_phi = x_52;
        if ((1 < i32(x_54))) {
        } else {
          break;
        }
        x_45_phi = 1.0;
        x_42_phi = true;
        break;

        continuing {
          x_50_phi = 0.0;
          x_9_phi = 0;
          x_52_phi = false;
        }
      }
      x_45 = x_45_phi;
      x_42 = x_42_phi;
      x_60_phi = x_45;
      x_61_phi = x_42;
      if (x_42) {
        break;
      }

      continuing {
        x_7 = bitcast<i32>((x_9 + bitcast<i32>(1)));
        x_41_phi = x_42;
        x_8_phi = x_7;
        x_44_phi = x_45;
      }
    }
    x_60 = x_60_phi;
    let x_61 : bool = x_61_phi;
    x_63_phi = x_60;
    if (x_61) {
      break;
    }
    x_63_phi = 1.0;
    break;
  }
  let x_63 : f32 = x_63_phi;
  return x_63;
}

fn main_1() {
  let x_34 : f32 = makeFrame_();
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
