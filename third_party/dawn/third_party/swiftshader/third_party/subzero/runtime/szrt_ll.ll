;;===- subzero/runtime/szrt_ll.ll - Subzero runtime source ----------------===;;
;;
;;                        The Subzero Code Generator
;;
;; This file is distributed under the University of Illinois Open Source
;; License. See LICENSE.TXT for details.
;;
;;===----------------------------------------------------------------------===;;
;;
;; This file implements wrappers for particular bitcode instructions that are
;; too uncommon and complex for a particular target to bother implementing
;; directly in Subzero target lowering.  This needs to be compiled by some
;; non-Subzero compiler.
;;
;;===----------------------------------------------------------------------===;;

define <4 x float> @__Sz_uitofp_4xi32_4xf32(<4 x i32> %a) {
entry:
  %0 = uitofp <4 x i32> %a to <4 x float>
  ret <4 x float> %0
}

define <4 x i32> @__Sz_fptoui_4xi32_f32(<4 x float> %a) {
entry:
  %0 = fptoui <4 x float> %a to <4 x i32>
  ret <4 x i32> %0
}

define i32 @__Sz_bitcast_8xi1_i8(<8 x i1> %a) {
entry:
  %0 = bitcast <8 x i1> %a to i8
  %ret = zext i8 %0 to i32
  ret i32 %ret
}

define i32 @__Sz_bitcast_16xi1_i16(<16 x i1> %a) {
entry:
  %0 = bitcast <16 x i1> %a to i16
  %ret = zext i16 %0 to i32
  ret i32 %ret
}

define <8 x i1> @__Sz_bitcast_i8_8xi1(i8 %a) {
entry:
  %0 = bitcast i8 %a to <8 x i1>
  ret <8 x i1> %0
}

define <16 x i1> @__Sz_bitcast_i16_16xi1(i16 %a) {
entry:
  %0 = bitcast i16 %a to <16 x i1>
  ret <16 x i1> %0
}
