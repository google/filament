struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 12u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn f_i1_(a : ptr<function, i32>) -> i32 {
  var i : i32;
  let x_16 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  i = x_16;
  loop {
    let x_17 : i32 = i;
    let x_18 : i32 = x_8.x_GLF_uniform_int_values[6].el;
    if ((x_17 < x_18)) {
    } else {
      break;
    }
    let x_19 : i32 = i;
    let x_20 : i32 = x_8.x_GLF_uniform_int_values[2].el;
    if ((x_19 > x_20)) {
      let x_21 : i32 = *(a);
      return x_21;
    }

    continuing {
      let x_22 : i32 = i;
      i = (x_22 + 1);
    }
  }
  let x_24 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  return x_24;
}

fn main_1() {
  var r : array<i32, 10u>;
  var i_1 : i32;
  var a_1 : array<i32, 10u>;
  var param : i32;
  var param_1 : i32;
  var i_2 : i32;
  let x_25 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  let x_26 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  r[x_25] = x_26;
  let x_27 : i32 = x_8.x_GLF_uniform_int_values[11].el;
  let x_28 : i32 = x_8.x_GLF_uniform_int_values[2].el;
  r[x_27] = x_28;
  let x_29 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  let x_30 : i32 = x_8.x_GLF_uniform_int_values[3].el;
  r[x_29] = x_30;
  let x_31 : i32 = x_8.x_GLF_uniform_int_values[2].el;
  let x_32 : i32 = x_8.x_GLF_uniform_int_values[4].el;
  r[x_31] = x_32;
  let x_33 : i32 = x_8.x_GLF_uniform_int_values[3].el;
  let x_34 : i32 = x_8.x_GLF_uniform_int_values[5].el;
  r[x_33] = x_34;
  let x_35 : i32 = x_8.x_GLF_uniform_int_values[4].el;
  let x_36 : i32 = x_8.x_GLF_uniform_int_values[6].el;
  r[x_35] = x_36;
  let x_37 : i32 = x_8.x_GLF_uniform_int_values[5].el;
  let x_38 : i32 = x_8.x_GLF_uniform_int_values[7].el;
  r[x_37] = x_38;
  let x_39 : i32 = x_8.x_GLF_uniform_int_values[8].el;
  let x_40 : i32 = x_8.x_GLF_uniform_int_values[8].el;
  r[x_39] = x_40;
  let x_41 : i32 = x_8.x_GLF_uniform_int_values[9].el;
  let x_42 : i32 = x_8.x_GLF_uniform_int_values[9].el;
  r[x_41] = x_42;
  let x_43 : i32 = x_8.x_GLF_uniform_int_values[10].el;
  let x_44 : i32 = x_8.x_GLF_uniform_int_values[10].el;
  r[x_43] = x_44;
  let x_45 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  i_1 = x_45;
  loop {
    let x_46 : i32 = i_1;
    let x_47 : i32 = x_8.x_GLF_uniform_int_values[6].el;
    if ((x_46 < x_47)) {
    } else {
      break;
    }
    let x_48 : i32 = i_1;
    let x_49 : i32 = i_1;
    a_1[x_48] = x_49;
    let x_50 : i32 = i_1;
    let x_51 : i32 = x_8.x_GLF_uniform_int_values[6].el;
    let x_52 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    if ((x_50 < (x_51 / x_52))) {
      let x_54 : i32 = i_1;
      let x_55 : i32 = i_1;
      let x_56 : i32 = x_8.x_GLF_uniform_int_values[1].el;
      a_1[x_54] = (x_55 + x_56);
      let x_58 : i32 = i_1;
      let x_59 : i32 = x_8.x_GLF_uniform_int_values[6].el;
      if ((x_58 < x_59)) {
        continue;
      }
      let x_60 : i32 = i_1;
      let x_61 : i32 = i_1;
      let x_62 : i32 = x_8.x_GLF_uniform_int_values[8].el;
      a_1[x_60] = (x_61 + x_62);
      let x_64 : i32 = i_1;
      let x_65 : i32 = a_1[x_64];
      param = x_65;
      let x_66 : i32 = f_i1_(&(param));
      let x_67 : i32 = x_8.x_GLF_uniform_int_values[8].el;
      if ((x_66 < x_67)) {
        let x_68 : i32 = i_1;
        let x_182_save = x_68;
        let x_69 : i32 = a_1[x_182_save];
        a_1[x_182_save] = (x_69 - 1);
      }
    } else {
      let x_71 : i32 = i_1;
      let x_72 : i32 = a_1[x_71];
      param_1 = x_72;
      let x_73 : i32 = f_i1_(&(param_1));
      let x_74 : i32 = x_8.x_GLF_uniform_int_values[8].el;
      if ((x_73 < x_74)) {
        let x_75 : i32 = i_1;
        let x_76 : i32 = x_8.x_GLF_uniform_int_values[4].el;
        let x_77 : i32 = a_1[x_75];
        a_1[x_75] = (x_77 + x_76);
      }
    }

    continuing {
      let x_79 : i32 = i_1;
      i_1 = (x_79 + 1);
    }
  }
  let x_81 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  i_2 = x_81;
  loop {
    let x_82 : i32 = i_2;
    let x_83 : i32 = x_8.x_GLF_uniform_int_values[6].el;
    if ((x_82 < x_83)) {
    } else {
      break;
    }
    let x_84 : i32 = i_2;
    let x_85 : i32 = a_1[x_84];
    let x_86 : i32 = i_2;
    let x_87 : i32 = r[x_86];
    if ((x_85 != x_87)) {
      let x_88 : i32 = x_8.x_GLF_uniform_int_values[0].el;
      let x_205 : f32 = f32(x_88);
      x_GLF_color = vec4<f32>(x_205, x_205, x_205, x_205);
      return;
    }

    continuing {
      let x_89 : i32 = i_2;
      i_2 = (x_89 + 1);
    }
  }
  let x_91 : i32 = x_8.x_GLF_uniform_int_values[11].el;
  let x_92 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  let x_93 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  let x_94 : i32 = x_8.x_GLF_uniform_int_values[11].el;
  x_GLF_color = vec4<f32>(f32(x_91), f32(x_92), f32(x_93), f32(x_94));
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
