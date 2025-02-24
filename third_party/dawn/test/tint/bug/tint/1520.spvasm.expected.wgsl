struct UniformBuffer {
  @size(16)
  padding_0 : u32,
  /* @offset(16) */
  unknownInput_S1_c0 : f32,
  /* @offset(32) */
  ucolorRed_S1_c0 : vec4f,
  /* @offset(48) */
  ucolorGreen_S1_c0 : vec4f,
  /* @offset(64) */
  umatrix_S1 : mat3x3f,
}

@binding(0) @group(0) var<uniform> x_4 : UniformBuffer;

var<private> sk_FragColor : vec4f;

var<private> sk_Clockwise : bool;

var<private> vcolor_S0 : vec4f;

const x_46 = vec4i(1i);

const x_57 = vec4i(2i);

fn test_int_S1_c0_b() -> bool {
  var unknown : i32;
  var ok : bool;
  var val : vec4i;
  var x_40 : bool;
  var x_41 : bool;
  var x_54 : bool;
  var x_55 : bool;
  var x_65 : bool;
  var x_66 : bool;
  let x_27 = i32(x_4.unknownInput_S1_c0);
  unknown = x_27;
  ok = true;
  x_41 = false;
  if (true) {
    x_40 = all(((vec4i() / vec4i(x_27)) == vec4i()));
    x_41 = x_40;
  }
  ok = x_41;
  let x_44 = vec4i(x_27);
  val = x_44;
  let x_47 = (x_44 + x_46);
  val = x_47;
  let x_48 = (x_47 - x_46);
  val = x_48;
  let x_49 = (x_48 + x_46);
  val = x_49;
  let x_50 = (x_49 - x_46);
  val = x_50;
  x_55 = false;
  if (x_41) {
    x_54 = all((x_50 == x_44));
    x_55 = x_54;
  }
  ok = x_55;
  let x_58 = (x_50 * x_57);
  val = x_58;
  let x_59 = (x_58 / x_57);
  val = x_59;
  let x_60 = (x_59 * x_57);
  val = x_60;
  let x_61 = (x_60 / x_57);
  val = x_61;
  x_66 = false;
  if (x_55) {
    x_65 = all((x_61 == x_44));
    x_66 = x_65;
  }
  ok = x_66;
  return x_66;
}

const x_91 = vec4f(1.0f);

const x_102 = vec4f(2.0f);

fn main_1() {
  var outputColor_S0 : vec4f;
  var output_S1 : vec4f;
  var x_8_unknown : f32;
  var x_9_ok : bool;
  var x_10_val : vec4f;
  var x_116 : vec4f;
  var x_86 : bool;
  var x_87 : bool;
  var x_99 : bool;
  var x_100 : bool;
  var x_110 : bool;
  var x_111 : bool;
  var x_114 : bool;
  var x_115 : bool;
  outputColor_S0 = vcolor_S0;
  let x_77 = x_4.unknownInput_S1_c0;
  x_8_unknown = x_77;
  x_9_ok = true;
  x_87 = false;
  if (true) {
    x_86 = all(((vec4f() / vec4f(x_77)) == vec4f()));
    x_87 = x_86;
  }
  x_9_ok = x_87;
  let x_89 = vec4f(x_77);
  x_10_val = x_89;
  let x_92 = (x_89 + x_91);
  x_10_val = x_92;
  let x_93 = (x_92 - x_91);
  x_10_val = x_93;
  let x_94 = (x_93 + x_91);
  x_10_val = x_94;
  let x_95 = (x_94 - x_91);
  x_10_val = x_95;
  x_100 = false;
  if (x_87) {
    x_99 = all((x_95 == x_89));
    x_100 = x_99;
  }
  x_9_ok = x_100;
  let x_103 = (x_95 * x_102);
  x_10_val = x_103;
  let x_104 = (x_103 / x_102);
  x_10_val = x_104;
  let x_105 = (x_104 * x_102);
  x_10_val = x_105;
  let x_106 = (x_105 / x_102);
  x_10_val = x_106;
  x_111 = false;
  if (x_100) {
    x_110 = all((x_106 == x_89));
    x_111 = x_110;
  }
  x_9_ok = x_111;
  x_115 = false;
  if (x_111) {
    x_114 = test_int_S1_c0_b();
    x_115 = x_114;
  }
  if (x_115) {
    x_116 = x_4.ucolorGreen_S1_c0;
  } else {
    x_116 = x_4.ucolorRed_S1_c0;
  }
  let x_125 = x_116;
  output_S1 = x_116;
  sk_FragColor = x_125;
  return;
}

struct main_out {
  @location(0)
  sk_FragColor_1 : vec4f,
}

@fragment
fn main(@builtin(front_facing) sk_Clockwise_param : bool, @location(0) vcolor_S0_param : vec4f) -> main_out {
  sk_Clockwise = sk_Clockwise_param;
  vcolor_S0 = vcolor_S0_param;
  main_1();
  return main_out(sk_FragColor);
}
