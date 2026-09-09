; RUN: %opt %s -dynamic-vector-to-array -S | FileCheck %s

; Make sure argument is cast before call.
; CHECK:%[[Cast:.+]] = bitcast [3 x [3 x float]]* %0 to [3 x <3 x float>]*
; CHECK:%call = call float @"\01?foo@@YAMY02V?$vector@M$02@@@Z"([3 x <3 x float>]* %[[Cast]])

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind
define float @"\01?bar@@YAMY02V?$vector@M$02@@@Z"([3 x <3 x float>]* %a) #0 {
entry:
  %0 = alloca [3 x <3 x float>]
  %1 = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %0, i32 0, i32 0
  %2 = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %a, i32 0, i32 0
  %3 = load <3 x float>, <3 x float>* %2
  store <3 x float> %3, <3 x float>* %1
  %4 = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %0, i32 0, i32 1
  %5 = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %a, i32 0, i32 1
  %6 = load <3 x float>, <3 x float>* %5
  store <3 x float> %6, <3 x float>* %4
  %7 = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %0, i32 0, i32 2
  %8 = getelementptr inbounds [3 x <3 x float>], [3 x <3 x float>]* %a, i32 0, i32 2
  %9 = load <3 x float>, <3 x float>* %8
  store <3 x float> %9, <3 x float>* %7
  %call = call float @"\01?foo@@YAMY02V?$vector@M$02@@@Z"([3 x <3 x float>]* %0)
  ret float %call
}

declare float @"\01?foo@@YAMY02V?$vector@M$02@@@Z"([3 x <3 x float>]*)

attributes #0 = { nounwind }
