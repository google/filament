; RUN: %dxopt %s -reassociate,EnableAggressiveReassociation=1 -gvn -S | FileCheck %s -check-prefixes=CHECK,COMMON_FACTOR
; RUN: %dxopt %s -reassociate,EnableAggressiveReassociation=0 -gvn -S | FileCheck %s -check-prefixes=CHECK,NO_COMMON_FACTOR

; CHECK: @test1

; COMMON_FACTOR:      %[[FACTOR:.*]] = mul i32 %X4, %X3
; COMMON_FACTOR-NEXT: %[[C:.*]] = mul i32 %[[FACTOR]], %X1
; COMMON_FACTOR-NEXT: %[[D:.*]] = mul i32 %[[FACTOR]], %X2

; NO_COMMON_FACTOR: %[[A:.*]] = mul i32 %X3, %X1
; NO_COMMON_FACTOR: %[[B:.*]] = mul i32 %X3, %X2
; NO_COMMON_FACTOR: %[[C:.*]] = mul i32 %[[A]], %X4
; NO_COMMON_FACTOR: %[[D:.*]] = mul i32 %[[B]], %X4

; CHECK: %[[E:.*]] = xor i32 %[[C]], %[[D]]
; CHECK: ret i32 %[[E]]
define i32 @test1(i32 %X1, i32 %X2, i32 %X3, i32 %X4) {
  %A = mul i32 %X3, %X1
  %B = mul i32 %X3, %X2
  %C = mul i32 %A, %X4
  %D = mul i32 %B, %X4
  %E = xor i32 %C, %D
  ret i32 %E
}