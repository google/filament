struct buf0 {
  injected : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

fn main_1() {
  var m : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  let x_30 : f32 = x_5.injected.x;
  let x_32 : f32 = x_5.injected.y;
  if ((x_30 > x_32)) {
    loop {

      continuing {
        break if !(false);
      }
    }
    m = 1;
    loop {
      if (true) {
      } else {
        break;
      }
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
