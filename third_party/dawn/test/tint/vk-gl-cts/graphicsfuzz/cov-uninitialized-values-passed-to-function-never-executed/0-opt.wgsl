struct S {
  data : i32,
}

struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_struct_S_i11_i1_(s : ptr<function, S>, x : ptr<function, i32>) {
  let x_103 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  let x_105 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  if ((x_103 == x_105)) {
    return;
  }
  let x_109 : i32 = *(x);
  (*(s)).data = x_109;
  return;
}

fn main_1() {
  var i : i32;
  var arr : array<S, 10u>;
  var index : i32;
  var param : S;
  var param_1 : i32;
  var param_2 : S;
  var param_3 : i32;
  i = 0;
  loop {
    let x_43 : i32 = i;
    if ((x_43 < 10)) {
    } else {
      break;
    }
    let x_46 : i32 = i;
    arr[x_46].data = 0;

    continuing {
      let x_48 : i32 = i;
      i = (x_48 + 1);
    }
  }
  let x_51 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  let x_53 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  if ((x_51 == x_53)) {
    let x_58 : i32 = index;
    let x_60 : S = arr[x_58];
    param = x_60;
    let x_61 : i32 = index;
    param_1 = x_61;
    func_struct_S_i11_i1_(&(param), &(param_1));
    let x_63 : S = param;
    arr[x_58] = x_63;
  } else {
    let x_66 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_68 : S = arr[x_66];
    param_2 = x_68;
    let x_70 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    param_3 = x_70;
    func_struct_S_i11_i1_(&(param_2), &(param_3));
    let x_72 : S = param_2;
    arr[x_66] = x_72;
  }
  let x_75 : i32 = x_9.x_GLF_uniform_int_values[0].el;
  let x_77 : i32 = arr[x_75].data;
  let x_79 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  if ((x_77 == x_79)) {
    let x_85 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_88 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_91 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_94 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_85), f32(x_88), f32(x_91), f32(x_94));
  } else {
    let x_98 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_99 : f32 = f32(x_98);
    x_GLF_color = vec4<f32>(x_99, x_99, x_99, x_99);
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
