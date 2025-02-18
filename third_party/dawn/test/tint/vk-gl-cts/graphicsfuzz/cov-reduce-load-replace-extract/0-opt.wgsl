struct S {
  x : i32,
  y : i32,
}

struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_struct_S_i1_i11_(arg : ptr<function, S>) {
  (*(arg)).y = 1;
  return;
}

fn main_1() {
  var a : f32;
  var b : array<S, 2u>;
  var param : S;
  a = 5.0;
  loop {
    let x_43 : i32 = x_10.one;
    b[x_43].x = 1;
    let x_46 : i32 = b[1].x;
    if ((x_46 == 1)) {
      let x_51 : i32 = x_10.one;
      if ((x_51 == 1)) {
        break;
      }
      let x_56 : S = b[1];
      param = x_56;
      func_struct_S_i1_i11_(&(param));
      let x_58 : S = param;
      b[1] = x_58;
      let x_61 : i32 = b[1].y;
      a = f32(x_61);
    }
    a = 0.0;

    continuing {
      break if !(false);
    }
  }
  let x_63 : f32 = a;
  if ((x_63 == 5.0)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
