struct S {
  f0 : i32,
  f1 : vec3<bool>,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var ll : S;
  var sums : array<f32, 9u>;
  ll = S(0, vec3<bool>(true, true, true));
  loop {
    let x_12 : S = ll;
    let x_45 : f32 = x_7.injectionSwitch.y;
    if ((x_12.f0 != i32(x_45))) {
    } else {
      break;
    }
    sums[0] = 0.0;

    continuing {
      let x_13 : S = ll;
      let x_50 : S = ll;
      var x_51_1 : S = x_50;
      x_51_1.f0 = (x_13.f0 + 1);
      let x_51 : S = x_51_1;
      ll = x_51;
    }
  }
  let x_53 : f32 = sums[0];
  let x_54 : vec2<f32> = vec2<f32>(x_53, x_53);
  x_GLF_color = vec4<f32>(1.0, x_54.x, x_54.y, 1.0);
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
