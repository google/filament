struct S {
  arr : array<i32, 2u>,
}

struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_struct_S_i1_2_1_i1_(s : ptr<function, S>, x : ptr<function, i32>) -> i32 {
  let x_16 : i32 = *(x);
  (*(s)).arr[1] = (x_16 + 1);
  let x_18 : i32 = x_9.one;
  let x_19 : i32 = (*(s)).arr[x_18];
  let x_20 : i32 = *(x);
  if ((x_19 == x_20)) {
    return -1;
  }
  let x_21 : i32 = *(x);
  return x_21;
}

fn main_1() {
  var a : i32;
  var i : i32;
  var j : i32;
  var s_1 : S;
  var param : S;
  var param_1 : i32;
  a = 0;
  i = 0;
  loop {
    let x_22 : i32 = i;
    let x_23 : i32 = x_9.one;
    if ((x_22 < (2 + x_23))) {
    } else {
      break;
    }
    j = 0;
    loop {
      let x_25 : i32 = j;
      let x_26 : i32 = x_9.one;
      if ((x_25 < (3 + x_26))) {
      } else {
        break;
      }
      let x_28 : i32 = i;
      let x_29 : i32 = j;
      let x_79 : S = s_1;
      param = x_79;
      param_1 = (x_28 + x_29);
      let x_31 : i32 = func_struct_S_i1_2_1_i1_(&(param), &(param_1));
      let x_32 : i32 = a;
      a = (x_32 + x_31);

      continuing {
        let x_34 : i32 = j;
        j = (x_34 + 1);
      }
    }

    continuing {
      let x_36 : i32 = i;
      i = (x_36 + 1);
    }
  }
  let x_38 : i32 = a;
  if ((x_38 == 30)) {
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
