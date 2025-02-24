define <4 x float> @_Z6selectDv4_iDv4_fS0_(<4 x i32> %cond.ext, <4 x float> %arg1, <4 x float> %arg2) {
entry:
  %cond = trunc <4 x i32> %cond.ext to <4 x i1>
  %res = select <4 x i1> %cond, <4 x float> %arg1, <4 x float> %arg2
  ret <4 x float> %res
}

define <4 x i32> @_Z6selectDv4_iS_S_(<4 x i32> %cond.ext, <4 x i32> %arg1, <4 x i32> %arg2) {
entry:
  %cond = trunc <4 x i32> %cond.ext to <4 x i1>
  %res = select <4 x i1> %cond, <4 x i32> %arg1, <4 x i32> %arg2
  ret <4 x i32> %res
}

define <4 x i32> @_Z6selectDv4_iDv4_jS0_(<4 x i32> %cond.ext, <4 x i32> %arg1, <4 x i32> %arg2) {
entry:
  %cond = trunc <4 x i32> %cond.ext to <4 x i1>
  %res = select <4 x i1> %cond, <4 x i32> %arg1, <4 x i32> %arg2
  ret <4 x i32> %res
}

define <8 x i16> @_Z6selectDv8_sS_S_(<8 x i16> %cond.ext, <8 x i16> %arg1, <8 x i16> %arg2) {
entry:
  %cond = trunc <8 x i16> %cond.ext to <8 x i1>
  %res = select <8 x i1> %cond, <8 x i16> %arg1, <8 x i16> %arg2
  ret <8 x i16> %res
}

define <8 x i16> @_Z6selectDv8_sDv8_tS0_(<8 x i16> %cond.ext, <8 x i16> %arg1, <8 x i16> %arg2) {
entry:
  %cond = trunc <8 x i16> %cond.ext to <8 x i1>
  %res = select <8 x i1> %cond, <8 x i16> %arg1, <8 x i16> %arg2
  ret <8 x i16> %res
}

define <16 x i8> @_Z6selectDv16_aS_S_(<16 x i8> %cond.ext, <16 x i8> %arg1, <16 x i8> %arg2) {
entry:
  %cond = trunc <16 x i8> %cond.ext to <16 x i1>
  %res = select <16 x i1> %cond, <16 x i8> %arg1, <16 x i8> %arg2
  ret <16 x i8> %res
}

define <16 x i8> @_Z6selectDv16_aDv16_hS0_(<16 x i8> %cond.ext, <16 x i8> %arg1, <16 x i8> %arg2) {
entry:
  %cond = trunc <16 x i8> %cond.ext to <16 x i1>
  %res = select <16 x i1> %cond, <16 x i8> %arg1, <16 x i8> %arg2
  ret <16 x i8> %res
}

define <4 x i32> @_Z9select_i1Dv4_iS_S_(<4 x i32> %cond.ext, <4 x i32> %arg1.ext, <4 x i32> %arg2.ext) {
entry:
  %cond = trunc <4 x i32> %cond.ext to <4 x i1>
  %arg1 = trunc <4 x i32> %arg1.ext to <4 x i1>
  %arg2 = trunc <4 x i32> %arg2.ext to <4 x i1>
  %res.trunc = select <4 x i1> %cond, <4 x i1> %arg1, <4 x i1> %arg2
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
}

define <8 x i16> @_Z9select_i1Dv8_sS_S_(<8 x i16> %cond.ext, <8 x i16> %arg1.ext, <8 x i16> %arg2.ext) {
entry:
  %cond = trunc <8 x i16> %cond.ext to <8 x i1>
  %arg1 = trunc <8 x i16> %arg1.ext to <8 x i1>
  %arg2 = trunc <8 x i16> %arg2.ext to <8 x i1>
  %res.trunc = select <8 x i1> %cond, <8 x i1> %arg1, <8 x i1> %arg2
  %res = sext <8 x i1> %res.trunc to <8 x i16>
  ret <8 x i16> %res
}

define <16 x i8> @_Z9select_i1Dv16_aS_S_(<16 x i8> %cond.ext, <16 x i8> %arg1.ext, <16 x i8> %arg2.ext) {
entry:
  %cond = trunc <16 x i8> %cond.ext to <16 x i1>
  %arg1 = trunc <16 x i8> %arg1.ext to <16 x i1>
  %arg2 = trunc <16 x i8> %arg2.ext to <16 x i1>
  %res.trunc = select <16 x i1> %cond, <16 x i1> %arg1, <16 x i1> %arg2
  %res = sext <16 x i1> %res.trunc to <16 x i8>
  ret <16 x i8> %res
}
