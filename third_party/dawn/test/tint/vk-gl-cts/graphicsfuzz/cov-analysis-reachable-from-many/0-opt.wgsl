struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> f32 {
  var s : f32;
  var i : i32;
  var j : i32;
  s = 2.0;
  i = 0;
  loop {
    let x_47 : i32 = i;
    let x_49 : i32 = x_8.zero;
    if ((x_47 < (x_49 + 1))) {
    } else {
      break;
    }
    let x_53 : f32 = s;
    s = (x_53 + 3.0);
    j = 0;
    loop {
      let x_59 : i32 = j;
      if ((x_59 < 10)) {
      } else {
        break;
      }
      let x_63 : i32 = x_8.zero;
      if ((x_63 == 1)) {
        discard;
      }

      continuing {
        let x_67 : i32 = j;
        j = (x_67 + 1);
      }
    }

    continuing {
      let x_69 : i32 = i;
      i = (x_69 + 1);
    }
  }
  let x_71 : f32 = s;
  return x_71;
}

fn main_1() {
  var c : vec4<f32>;
  let x_34 : f32 = func_();
  c = vec4<f32>(x_34, 0.0, 0.0, 1.0);
  let x_36 : f32 = func_();
  if ((x_36 == 5.0)) {
    let x_41 : vec4<f32> = c;
    x_GLF_color = x_41;
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
