struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

fn computeColor_() -> vec3<f32> {
  var x_injected_loop_counter : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  x_injected_loop_counter = 1;
  loop {
    let x_38 : f32 = x_7.injectionSwitch.x;
    if ((x_38 > 1.0)) {
      let x_43 : f32 = x_7.injectionSwitch.x;
      if ((x_43 > 1.0)) {
        continue;
      } else {
        continue;
      }
    }
    return vec3<f32>(1.0, 1.0, 1.0);
  }
}

fn main_1() {
  let x_31 : vec3<f32> = computeColor_();
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
