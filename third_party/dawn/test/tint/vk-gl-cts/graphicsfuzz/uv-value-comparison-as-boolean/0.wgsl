var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var c1 : bool;
  var uv : vec2<f32>;
  var i : i32;
  var x_37 : bool;
  var x_37_phi : bool;
  var x_9_phi : i32;
  let x_34 : f32 = uv.y;
  let x_35 : bool = (x_34 < 0.25);
  c1 = x_35;
  i = 0;
  x_37_phi = x_35;
  x_9_phi = 0;
  loop {
    x_37 = x_37_phi;
    let x_9 : i32 = x_9_phi;
    if ((x_9 < 1)) {
    } else {
      break;
    }
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    return;

    continuing {
      let x_7 : i32 = i;
      i = (x_7 + 1);
      x_37_phi = false;
      x_9_phi = 0;
    }
  }
  if (x_37) {
    return;
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
