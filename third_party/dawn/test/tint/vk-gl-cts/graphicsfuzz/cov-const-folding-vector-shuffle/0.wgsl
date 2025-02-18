var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_26 : vec2<f32>;
  var x_39 : bool;
  var x_26_phi : vec2<f32>;
  var x_5_phi : i32;
  var x_40_phi : bool;
  x_26_phi = vec2<f32>();
  x_5_phi = 2;
  loop {
    var x_27 : vec2<f32>;
    var x_4 : i32;
    x_26 = x_26_phi;
    let x_5 : i32 = x_5_phi;
    if ((x_5 < 3)) {
    } else {
      break;
    }

    continuing {
      let x_32 : vec2<f32> = vec2<f32>(1.0, f32(x_5));
      x_27 = vec2<f32>(x_32.x, x_32.y);
      x_4 = (x_5 + 1);
      x_26_phi = x_27;
      x_5_phi = x_4;
    }
  }
  let x_34 : bool = (x_26.x != 1.0);
  x_40_phi = x_34;
  if (!(x_34)) {
    x_39 = (x_26.y != 2.0);
    x_40_phi = x_39;
  }
  let x_40 : bool = x_40_phi;
  if (x_40) {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  } else {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
