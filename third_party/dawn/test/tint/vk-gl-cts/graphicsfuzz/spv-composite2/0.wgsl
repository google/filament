struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var indexable : array<vec4<f32>, 16u>;
  var x_66 : i32;
  var x_66_phi : i32;
  var x_69_phi : i32;
  let x_52 : vec4<f32> = gl_FragCoord;
  let x_55 : vec2<f32> = x_6.resolution;
  let x_56 : vec2<f32> = (vec2<f32>(x_52.x, x_52.y) / x_55);
  let x_64 : i32 = (i32((x_56.x * 10.0)) + (i32((x_56.y * 10.0)) * 10));
  x_66_phi = 100;
  x_69_phi = 0;
  loop {
    var x_67 : i32;
    var x_70 : i32;
    x_66 = x_66_phi;
    let x_69 : i32 = x_69_phi;
    if ((x_69 < x_64)) {
    } else {
      break;
    }

    continuing {
      x_67 = (((4 * bitcast<i32>(x_66)) * (1000 - bitcast<i32>(x_66))) / 1000);
      x_70 = (x_69 + 1);
      x_66_phi = x_67;
      x_69_phi = x_70;
    }
  }
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_78 : vec4<f32> = indexable[bitcast<i32>((x_66 % 16))];
  x_GLF_color = x_78;
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
