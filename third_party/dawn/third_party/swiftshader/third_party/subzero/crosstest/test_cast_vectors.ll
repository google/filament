define <4 x float> @_Z4castIDv4_iDv4_fET0_T_(<4 x i32> %a) {
entry:
  %0 = sitofp <4 x i32> %a to <4 x float>
  ret <4 x float> %0
}

define <4 x i32> @_Z4castIDv4_fDv4_iET0_T_(<4 x float> %a) {
entry:
  %0 = fptosi <4 x float> %a to <4 x i32>
  ret <4 x i32> %0
}

define <4 x float> @_Z4castIDv4_jDv4_fET0_T_(<4 x i32> %a) {
entry:
  %0 = uitofp <4 x i32> %a to <4 x float>
  ret <4 x float> %0
}

define <4 x i32> @_Z4castIDv4_fDv4_jET0_T_(<4 x float> %a) {
entry:
  %0 = fptoui <4 x float> %a to <4 x i32>
  ret <4 x i32> %0
}
