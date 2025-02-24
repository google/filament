define <4 x float> @insertelement_v4f32(<4 x float> %vec, float %elt, i32 %idx) {
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  ]
idx0:
  %res0 = insertelement <4 x float> %vec, float %elt, i32 0
  ret <4 x float> %res0
idx1:
  %res1 = insertelement <4 x float> %vec, float %elt, i32 1
  ret <4 x float> %res1
idx2:
  %res2 = insertelement <4 x float> %vec, float %elt, i32 2
  ret <4 x float> %res2
idx3:
  %res3 = insertelement <4 x float> %vec, float %elt, i32 3
  ret <4 x float> %res3
abort:
  unreachable
}

define <4 x i32> @insertelement_v4i1(<4 x i32> %arg_vec, i64 %elt_arg, i32 %idx) {
  %vec = trunc <4 x i32> %arg_vec to <4 x i1>
  %elt = trunc i64 %elt_arg to i1
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  ]
idx0:
  %res0_i1 = insertelement <4 x i1> %vec, i1 %elt, i32 0
  %res0 = zext <4 x i1> %res0_i1 to <4 x i32>
  ret <4 x i32> %res0
idx1:
  %res1_i1 = insertelement <4 x i1> %vec, i1 %elt, i32 1
  %res1 = zext <4 x i1> %res1_i1 to <4 x i32>
  ret <4 x i32> %res1
idx2:
  %res2_i1 = insertelement <4 x i1> %vec, i1 %elt, i32 2
  %res2 = zext <4 x i1> %res2_i1 to <4 x i32>
  ret <4 x i32> %res2
idx3:
  %res3_i1 = insertelement <4 x i1> %vec, i1 %elt, i32 3
  %res3 = zext <4 x i1> %res3_i1 to <4 x i32>
  ret <4 x i32> %res3
abort:
  unreachable
}

define <8 x i16> @insertelement_v8i1(<8 x i16> %arg_vec, i64 %elt_arg, i32 %idx) {
  %vec = trunc <8 x i16> %arg_vec to <8 x i1>
  %elt = trunc i64 %elt_arg to i1
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  i32 4, label %idx4
  i32 5, label %idx5
  i32 6, label %idx6
  i32 7, label %idx7
  ]
idx0:
  %res0_i1 = insertelement <8 x i1> %vec, i1 %elt, i32 0
  %res0 = zext <8 x i1> %res0_i1 to <8 x i16>
  ret <8 x i16> %res0
idx1:
  %res1_i1 = insertelement <8 x i1> %vec, i1 %elt, i32 1
  %res1 = zext <8 x i1> %res1_i1 to <8 x i16>
  ret <8 x i16> %res1
idx2:
  %res2_i1 = insertelement <8 x i1> %vec, i1 %elt, i32 2
  %res2 = zext <8 x i1> %res2_i1 to <8 x i16>
  ret <8 x i16> %res2
idx3:
  %res3_i1 = insertelement <8 x i1> %vec, i1 %elt, i32 3
  %res3 = zext <8 x i1> %res3_i1 to <8 x i16>
  ret <8 x i16> %res3
idx4:
  %res4_i1 = insertelement <8 x i1> %vec, i1 %elt, i32 4
  %res4 = zext <8 x i1> %res4_i1 to <8 x i16>
  ret <8 x i16> %res4
idx5:
  %res5_i1 = insertelement <8 x i1> %vec, i1 %elt, i32 5
  %res5 = zext <8 x i1> %res5_i1 to <8 x i16>
  ret <8 x i16> %res5
idx6:
  %res6_i1 = insertelement <8 x i1> %vec, i1 %elt, i32 6
  %res6 = zext <8 x i1> %res6_i1 to <8 x i16>
  ret <8 x i16> %res6
idx7:
  %res7_i1 = insertelement <8 x i1> %vec, i1 %elt, i32 7
  %res7 = zext <8 x i1> %res7_i1 to <8 x i16>
  ret <8 x i16> %res7
abort:
  unreachable
}

define <16 x i8> @insertelement_v16i1(<16 x i8> %arg_vec, i64 %elt_arg, i32 %idx) {
  %vec = trunc <16 x i8> %arg_vec to <16 x i1>
  %elt = trunc i64 %elt_arg to i1
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  i32 4, label %idx4
  i32 5, label %idx5
  i32 6, label %idx6
  i32 7, label %idx7
  i32 8, label %idx8
  i32 9, label %idx9
  i32 10, label %idx10
  i32 11, label %idx11
  i32 12, label %idx12
  i32 13, label %idx13
  i32 14, label %idx14
  i32 15, label %idx15
  ]
idx0:
  %res0_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 0
  %res0 = zext <16 x i1> %res0_i1 to <16 x i8>
  ret <16 x i8> %res0
idx1:
  %res1_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 1
  %res1 = zext <16 x i1> %res1_i1 to <16 x i8>
  ret <16 x i8> %res1
idx2:
  %res2_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 2
  %res2 = zext <16 x i1> %res2_i1 to <16 x i8>
  ret <16 x i8> %res2
idx3:
  %res3_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 3
  %res3 = zext <16 x i1> %res3_i1 to <16 x i8>
  ret <16 x i8> %res3
idx4:
  %res4_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 4
  %res4 = zext <16 x i1> %res4_i1 to <16 x i8>
  ret <16 x i8> %res4
idx5:
  %res5_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 5
  %res5 = zext <16 x i1> %res5_i1 to <16 x i8>
  ret <16 x i8> %res5
idx6:
  %res6_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 6
  %res6 = zext <16 x i1> %res6_i1 to <16 x i8>
  ret <16 x i8> %res6
idx7:
  %res7_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 7
  %res7 = zext <16 x i1> %res7_i1 to <16 x i8>
  ret <16 x i8> %res7
idx8:
  %res8_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 8
  %res8 = zext <16 x i1> %res8_i1 to <16 x i8>
  ret <16 x i8> %res8
idx9:
  %res9_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 9
  %res9 = zext <16 x i1> %res9_i1 to <16 x i8>
  ret <16 x i8> %res9
idx10:
  %res10_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 10
  %res10 = zext <16 x i1> %res10_i1 to <16 x i8>
  ret <16 x i8> %res10
idx11:
  %res11_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 11
  %res11 = zext <16 x i1> %res11_i1 to <16 x i8>
  ret <16 x i8> %res11
idx12:
  %res12_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 12
  %res12 = zext <16 x i1> %res12_i1 to <16 x i8>
  ret <16 x i8> %res12
idx13:
  %res13_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 13
  %res13 = zext <16 x i1> %res13_i1 to <16 x i8>
  ret <16 x i8> %res13
idx14:
  %res14_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 14
  %res14 = zext <16 x i1> %res14_i1 to <16 x i8>
  ret <16 x i8> %res14
idx15:
  %res15_i1 = insertelement <16 x i1> %vec, i1 %elt, i32 15
  %res15 = zext <16 x i1> %res15_i1 to <16 x i8>
  ret <16 x i8> %res15
abort:
  unreachable
}

define <4 x i32> @insertelement_v4si32(<4 x i32> %vec, i64 %elt_arg, i32 %idx) {
entry:
  %elt = trunc i64 %elt_arg to i32
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  ]
idx0:
  %res0 = insertelement <4 x i32> %vec, i32 %elt, i32 0
  ret <4 x i32> %res0
idx1:
  %res1 = insertelement <4 x i32> %vec, i32 %elt, i32 1
  ret <4 x i32> %res1
idx2:
  %res2 = insertelement <4 x i32> %vec, i32 %elt, i32 2
  ret <4 x i32> %res2
idx3:
  %res3 = insertelement <4 x i32> %vec, i32 %elt, i32 3
  ret <4 x i32> %res3
abort:
  unreachable
}

define <4 x i32> @insertelement_v4ui32(<4 x i32> %vec, i64 %elt_arg, i32 %idx) {
entry:
  %res = call <4 x i32> @insertelement_v4si32(<4 x i32> %vec, i64 %elt_arg, i32 %idx)
  ret <4 x i32> %res
}

define <8 x i16> @insertelement_v8si16(<8 x i16> %vec, i64 %elt_arg, i32 %idx) {
entry:
  %elt = trunc i64 %elt_arg to i16
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  i32 4, label %idx4
  i32 5, label %idx5
  i32 6, label %idx6
  i32 7, label %idx7
  ]
idx0:
  %res0 = insertelement <8 x i16> %vec, i16 %elt, i32 0
  ret <8 x i16> %res0
idx1:
  %res1 = insertelement <8 x i16> %vec, i16 %elt, i32 1
  ret <8 x i16> %res1
idx2:
  %res2 = insertelement <8 x i16> %vec, i16 %elt, i32 2
  ret <8 x i16> %res2
idx3:
  %res3 = insertelement <8 x i16> %vec, i16 %elt, i32 3
  ret <8 x i16> %res3
idx4:
  %res4 = insertelement <8 x i16> %vec, i16 %elt, i32 4
  ret <8 x i16> %res4
idx5:
  %res5 = insertelement <8 x i16> %vec, i16 %elt, i32 5
  ret <8 x i16> %res5
idx6:
  %res6 = insertelement <8 x i16> %vec, i16 %elt, i32 6
  ret <8 x i16> %res6
idx7:
  %res7 = insertelement <8 x i16> %vec, i16 %elt, i32 7
  ret <8 x i16> %res7
abort:
  unreachable
}

define <8 x i16> @insertelement_v8ui16(<8 x i16> %vec, i64 %elt_arg, i32 %idx) {
entry:
  %res = call <8 x i16> @insertelement_v8si16(<8 x i16> %vec, i64 %elt_arg, i32 %idx)
  ret <8 x i16> %res
}

define <16 x i8> @insertelement_v16si8(<16 x i8> %vec, i64 %elt_arg, i32 %idx) {
entry:
  %elt = trunc i64 %elt_arg to i8
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  i32 4, label %idx4
  i32 5, label %idx5
  i32 6, label %idx6
  i32 7, label %idx7
  i32 8, label %idx8
  i32 9, label %idx9
  i32 10, label %idx10
  i32 11, label %idx11
  i32 12, label %idx12
  i32 13, label %idx13
  i32 14, label %idx14
  i32 15, label %idx15
  ]
idx0:
  %res0 = insertelement <16 x i8> %vec, i8 %elt, i32 0
  ret <16 x i8> %res0
idx1:
  %res1 = insertelement <16 x i8> %vec, i8 %elt, i32 1
  ret <16 x i8> %res1
idx2:
  %res2 = insertelement <16 x i8> %vec, i8 %elt, i32 2
  ret <16 x i8> %res2
idx3:
  %res3 = insertelement <16 x i8> %vec, i8 %elt, i32 3
  ret <16 x i8> %res3
idx4:
  %res4 = insertelement <16 x i8> %vec, i8 %elt, i32 4
  ret <16 x i8> %res4
idx5:
  %res5 = insertelement <16 x i8> %vec, i8 %elt, i32 5
  ret <16 x i8> %res5
idx6:
  %res6 = insertelement <16 x i8> %vec, i8 %elt, i32 6
  ret <16 x i8> %res6
idx7:
  %res7 = insertelement <16 x i8> %vec, i8 %elt, i32 7
  ret <16 x i8> %res7
idx8:
  %res8 = insertelement <16 x i8> %vec, i8 %elt, i32 8
  ret <16 x i8> %res8
idx9:
  %res9 = insertelement <16 x i8> %vec, i8 %elt, i32 9
  ret <16 x i8> %res9
idx10:
  %res10 = insertelement <16 x i8> %vec, i8 %elt, i32 10
  ret <16 x i8> %res10
idx11:
  %res11 = insertelement <16 x i8> %vec, i8 %elt, i32 11
  ret <16 x i8> %res11
idx12:
  %res12 = insertelement <16 x i8> %vec, i8 %elt, i32 12
  ret <16 x i8> %res12
idx13:
  %res13 = insertelement <16 x i8> %vec, i8 %elt, i32 13
  ret <16 x i8> %res13
idx14:
  %res14 = insertelement <16 x i8> %vec, i8 %elt, i32 14
  ret <16 x i8> %res14
idx15:
  %res15 = insertelement <16 x i8> %vec, i8 %elt, i32 15
  ret <16 x i8> %res15
abort:
  unreachable
}

define <16 x i8> @insertelement_v16ui8(<16 x i8> %vec, i64 %elt_arg, i32 %idx) {
entry:
  %res = call <16 x i8> @insertelement_v16si8(<16 x i8> %vec, i64 %elt_arg, i32 %idx)
  ret <16 x i8> %res
}

define float @extractelement_v4f32(<4 x float> %vec, i32 %idx) {
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  ]
idx0:
  %res0 = extractelement <4 x float> %vec, i32 0
  ret float %res0
idx1:
  %res1 = extractelement <4 x float> %vec, i32 1
  ret float %res1
idx2:
  %res2 = extractelement <4 x float> %vec, i32 2
  ret float %res2
idx3:
  %res3 = extractelement <4 x float> %vec, i32 3
  ret float %res3
abort:
  unreachable
}

define i64 @extractelement_v4i1(<4 x i32> %arg_vec, i32 %idx) {
  %vec = trunc <4 x i32> %arg_vec to <4 x i1>
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  ]
idx0:
  %res0_i1 = extractelement <4 x i1> %vec, i32 0
  %res0 = zext i1 %res0_i1 to i64
  ret i64 %res0
idx1:
  %res1_i1 = extractelement <4 x i1> %vec, i32 1
  %res1 = zext i1 %res1_i1 to i64
  ret i64 %res1
idx2:
  %res2_i1 = extractelement <4 x i1> %vec, i32 2
  %res2 = zext i1 %res2_i1 to i64
  ret i64 %res2
idx3:
  %res3_i1 = extractelement <4 x i1> %vec, i32 3
  %res3 = zext i1 %res3_i1 to i64
  ret i64 %res3
abort:
  unreachable
}

define i64 @extractelement_v8i1(<8 x i16> %arg_vec, i32 %idx) {
  %vec = trunc <8 x i16> %arg_vec to <8 x i1>
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  i32 4, label %idx4
  i32 5, label %idx5
  i32 6, label %idx6
  i32 7, label %idx7
  ]
idx0:
  %res0_i1 = extractelement <8 x i1> %vec, i32 0
  %res0 = zext i1 %res0_i1 to i64
  ret i64 %res0
idx1:
  %res1_i1 = extractelement <8 x i1> %vec, i32 1
  %res1 = zext i1 %res1_i1 to i64
  ret i64 %res1
idx2:
  %res2_i1 = extractelement <8 x i1> %vec, i32 2
  %res2 = zext i1 %res2_i1 to i64
  ret i64 %res2
idx3:
  %res3_i1 = extractelement <8 x i1> %vec, i32 3
  %res3 = zext i1 %res3_i1 to i64
  ret i64 %res3
idx4:
  %res4_i1 = extractelement <8 x i1> %vec, i32 4
  %res4 = zext i1 %res4_i1 to i64
  ret i64 %res4
idx5:
  %res5_i1 = extractelement <8 x i1> %vec, i32 5
  %res5 = zext i1 %res5_i1 to i64
  ret i64 %res5
idx6:
  %res6_i1 = extractelement <8 x i1> %vec, i32 6
  %res6 = zext i1 %res6_i1 to i64
  ret i64 %res6
idx7:
  %res7_i1 = extractelement <8 x i1> %vec, i32 7
  %res7 = zext i1 %res7_i1 to i64
  ret i64 %res7
abort:
  unreachable
}

define i64 @extractelement_v16i1(<16 x i8> %arg_vec, i32 %idx) {
  %vec = trunc <16 x i8> %arg_vec to <16 x i1>
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  i32 4, label %idx4
  i32 5, label %idx5
  i32 6, label %idx6
  i32 7, label %idx7
  i32 8, label %idx8
  i32 9, label %idx9
  i32 10, label %idx10
  i32 11, label %idx11
  i32 12, label %idx12
  i32 13, label %idx13
  i32 14, label %idx14
  i32 15, label %idx15
  ]
idx0:
  %res0_i1 = extractelement <16 x i1> %vec, i32 0
  %res0 = zext i1 %res0_i1 to i64
  ret i64 %res0
idx1:
  %res1_i1 = extractelement <16 x i1> %vec, i32 1
  %res1 = zext i1 %res1_i1 to i64
  ret i64 %res1
idx2:
  %res2_i1 = extractelement <16 x i1> %vec, i32 2
  %res2 = zext i1 %res2_i1 to i64
  ret i64 %res2
idx3:
  %res3_i1 = extractelement <16 x i1> %vec, i32 3
  %res3 = zext i1 %res3_i1 to i64
  ret i64 %res3
idx4:
  %res4_i1 = extractelement <16 x i1> %vec, i32 4
  %res4 = zext i1 %res4_i1 to i64
  ret i64 %res4
idx5:
  %res5_i1 = extractelement <16 x i1> %vec, i32 5
  %res5 = zext i1 %res5_i1 to i64
  ret i64 %res5
idx6:
  %res6_i1 = extractelement <16 x i1> %vec, i32 6
  %res6 = zext i1 %res6_i1 to i64
  ret i64 %res6
idx7:
  %res7_i1 = extractelement <16 x i1> %vec, i32 7
  %res7 = zext i1 %res7_i1 to i64
  ret i64 %res7
idx8:
  %res8_i1 = extractelement <16 x i1> %vec, i32 8
  %res8 = zext i1 %res8_i1 to i64
  ret i64 %res8
idx9:
  %res9_i1 = extractelement <16 x i1> %vec, i32 9
  %res9 = zext i1 %res9_i1 to i64
  ret i64 %res9
idx10:
  %res10_i1 = extractelement <16 x i1> %vec, i32 10
  %res10 = zext i1 %res10_i1 to i64
  ret i64 %res10
idx11:
  %res11_i1 = extractelement <16 x i1> %vec, i32 11
  %res11 = zext i1 %res11_i1 to i64
  ret i64 %res11
idx12:
  %res12_i1 = extractelement <16 x i1> %vec, i32 12
  %res12 = zext i1 %res12_i1 to i64
  ret i64 %res12
idx13:
  %res13_i1 = extractelement <16 x i1> %vec, i32 13
  %res13 = zext i1 %res13_i1 to i64
  ret i64 %res13
idx14:
  %res14_i1 = extractelement <16 x i1> %vec, i32 14
  %res14 = zext i1 %res14_i1 to i64
  ret i64 %res14
idx15:
  %res15_i1 = extractelement <16 x i1> %vec, i32 15
  %res15 = zext i1 %res15_i1 to i64
  ret i64 %res15
abort:
  unreachable
}

define i64 @extractelement_v4si32(<4 x i32> %vec, i32 %idx) {
entry:
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  ]
idx0:
  %res0_i32 = extractelement <4 x i32> %vec, i32 0
  %res0 = zext i32 %res0_i32 to i64
  ret i64 %res0
idx1:
  %res1_i32 = extractelement <4 x i32> %vec, i32 1
  %res1 = zext i32 %res1_i32 to i64
  ret i64 %res1
idx2:
  %res2_i32 = extractelement <4 x i32> %vec, i32 2
  %res2 = zext i32 %res2_i32 to i64
  ret i64 %res2
idx3:
  %res3_i32 = extractelement <4 x i32> %vec, i32 3
  %res3 = zext i32 %res3_i32 to i64
  ret i64 %res3
abort:
  unreachable
}

define i64 @extractelement_v4ui32(<4 x i32> %vec, i32 %idx) {
entry:
  %res = call i64 @extractelement_v4si32(<4 x i32> %vec, i32 %idx)
  ret i64 %res
}

define i64 @extractelement_v8si16(<8 x i16> %vec, i32 %idx) {
entry:
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  i32 4, label %idx4
  i32 5, label %idx5
  i32 6, label %idx6
  i32 7, label %idx7
  ]
idx0:
  %res0_i16 = extractelement <8 x i16> %vec, i32 0
  %res0 = zext i16 %res0_i16 to i64
  ret i64 %res0
idx1:
  %res1_i16 = extractelement <8 x i16> %vec, i32 1
  %res1 = zext i16 %res1_i16 to i64
  ret i64 %res1
idx2:
  %res2_i16 = extractelement <8 x i16> %vec, i32 2
  %res2 = zext i16 %res2_i16 to i64
  ret i64 %res2
idx3:
  %res3_i16 = extractelement <8 x i16> %vec, i32 3
  %res3 = zext i16 %res3_i16 to i64
  ret i64 %res3
idx4:
  %res4_i16 = extractelement <8 x i16> %vec, i32 4
  %res4 = zext i16 %res4_i16 to i64
  ret i64 %res4
idx5:
  %res5_i16 = extractelement <8 x i16> %vec, i32 5
  %res5 = zext i16 %res5_i16 to i64
  ret i64 %res5
idx6:
  %res6_i16 = extractelement <8 x i16> %vec, i32 6
  %res6 = zext i16 %res6_i16 to i64
  ret i64 %res6
idx7:
  %res7_i16 = extractelement <8 x i16> %vec, i32 7
  %res7 = zext i16 %res7_i16 to i64
  ret i64 %res7
abort:
  unreachable
}

define i64 @extractelement_v8ui16(<8 x i16> %vec, i32 %idx) {
entry:
  %res = call i64 @extractelement_v8si16(<8 x i16> %vec, i32 %idx)
  ret i64 %res
}

define i64 @extractelement_v16si8(<16 x i8> %vec, i32 %idx) {
entry:
  switch i32 %idx, label %abort [
  i32 0, label %idx0
  i32 1, label %idx1
  i32 2, label %idx2
  i32 3, label %idx3
  i32 4, label %idx4
  i32 5, label %idx5
  i32 6, label %idx6
  i32 7, label %idx7
  i32 8, label %idx8
  i32 9, label %idx9
  i32 10, label %idx10
  i32 11, label %idx11
  i32 12, label %idx12
  i32 13, label %idx13
  i32 14, label %idx14
  i32 15, label %idx15
  ]
idx0:
  %res0_i8 = extractelement <16 x i8> %vec, i32 0
  %res0 = zext i8 %res0_i8 to i64
  ret i64 %res0
idx1:
  %res1_i8 = extractelement <16 x i8> %vec, i32 1
  %res1 = zext i8 %res1_i8 to i64
  ret i64 %res1
idx2:
  %res2_i8 = extractelement <16 x i8> %vec, i32 2
  %res2 = zext i8 %res2_i8 to i64
  ret i64 %res2
idx3:
  %res3_i8 = extractelement <16 x i8> %vec, i32 3
  %res3 = zext i8 %res3_i8 to i64
  ret i64 %res3
idx4:
  %res4_i8 = extractelement <16 x i8> %vec, i32 4
  %res4 = zext i8 %res4_i8 to i64
  ret i64 %res4
idx5:
  %res5_i8 = extractelement <16 x i8> %vec, i32 5
  %res5 = zext i8 %res5_i8 to i64
  ret i64 %res5
idx6:
  %res6_i8 = extractelement <16 x i8> %vec, i32 6
  %res6 = zext i8 %res6_i8 to i64
  ret i64 %res6
idx7:
  %res7_i8 = extractelement <16 x i8> %vec, i32 7
  %res7 = zext i8 %res7_i8 to i64
  ret i64 %res7
idx8:
  %res8_i8 = extractelement <16 x i8> %vec, i32 8
  %res8 = zext i8 %res8_i8 to i64
  ret i64 %res8
idx9:
  %res9_i8 = extractelement <16 x i8> %vec, i32 9
  %res9 = zext i8 %res9_i8 to i64
  ret i64 %res9
idx10:
  %res10_i8 = extractelement <16 x i8> %vec, i32 10
  %res10 = zext i8 %res10_i8 to i64
  ret i64 %res10
idx11:
  %res11_i8 = extractelement <16 x i8> %vec, i32 11
  %res11 = zext i8 %res11_i8 to i64
  ret i64 %res11
idx12:
  %res12_i8 = extractelement <16 x i8> %vec, i32 12
  %res12 = zext i8 %res12_i8 to i64
  ret i64 %res12
idx13:
  %res13_i8 = extractelement <16 x i8> %vec, i32 13
  %res13 = zext i8 %res13_i8 to i64
  ret i64 %res13
idx14:
  %res14_i8 = extractelement <16 x i8> %vec, i32 14
  %res14 = zext i8 %res14_i8 to i64
  ret i64 %res14
idx15:
  %res15_i8 = extractelement <16 x i8> %vec, i32 15
  %res15 = zext i8 %res15_i8 to i64
  ret i64 %res15
abort:
  unreachable
}

define i64 @extractelement_v16ui8(<16 x i8> %vec, i32 %idx) {
entry:
  %res = call i64 @extractelement_v16si8(<16 x i8> %vec, i32 %idx)
  ret i64 %res
}
