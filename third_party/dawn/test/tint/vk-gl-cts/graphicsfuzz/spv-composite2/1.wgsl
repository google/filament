struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var indexable : array<vec4<f32>, 16u>;
  var x_72 : i32;
  var x_72_phi : i32;
  var x_75_phi : i32;
  let x_54 : vec4<f32> = gl_FragCoord;
  let x_55 : vec2<f32> = vec2<f32>(x_54.x, x_54.y);
  let x_58 : vec2<f32> = x_6.resolution;
  let x_59 : vec2<f32> = (x_55 / x_58);
  let x_70 : i32 = (i32((x_59.x * vec4<f32>(vec4<f32>(0.0, x_55, 0.5).w, 10.0, vec2<f32>(0.0, 0.0)).y)) + (i32((x_59.y * 10.0)) * 10));
  x_72_phi = 100;
  x_75_phi = 0;
  loop {
    var x_73 : i32;
    var x_76 : i32;
    x_72 = x_72_phi;
    let x_75 : i32 = x_75_phi;
    if ((x_75 < x_70)) {
    } else {
      break;
    }

    continuing {
      x_73 = (((4 * bitcast<i32>(x_72)) * (1000 - bitcast<i32>(x_72))) / 1000);
      x_76 = (x_75 + 1);
      x_72_phi = x_73;
      x_75_phi = x_76;
    }
  }
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_84 : vec4<f32> = indexable[bitcast<i32>((x_72 % 16))];
  x_GLF_color = x_84;
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
