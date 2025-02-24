struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn performPartition_() -> i32 {
  var GLF_live0i : i32;
  var i : i32;
  var x_11 : i32;
  var x_10_phi : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  x_10_phi = 0;
  loop {
    var x_11_phi : i32;
    let x_10 : i32 = x_10_phi;
    var x_42 : bool;
    let x_41 : f32 = x_6.injectionSwitch.y;
    x_42 = (x_41 < 0.0);
    if (x_42) {
      x_11_phi = x_10;
      continue;
    } else {
      GLF_live0i = 0;
      loop {
        let x_47 : bool = (0 < 1);
        if (x_42) {
          break;
        }
        return 1;
      }
      if (x_42) {
        loop {
          return 1;
        }
      }
      x_11_phi = x_10;
      continue;
    }

    continuing {
      x_11 = x_11_phi;
      x_10_phi = x_11;
      break if !(false);
    }
  }
  return x_11;
}

fn main_1() {
  let x_9 : i32 = performPartition_();
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
