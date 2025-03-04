; RUN: llc < %s -march=nvptx -mcpu=sm_20 | FileCheck %s
; RUN: llc < %s -march=nvptx64 -mcpu=sm_20 | FileCheck %s

define ptx_device float @test_fabsf(float %f) {
; CHECK: abs.f32 %f{{[0-9]+}}, %f{{[0-9]+}};
; CHECK: ret;
	%x = call float @llvm.fabs.f32(float %f)
	ret float %x
}

define ptx_device double @test_fabs(double %d) {
; CHECK: abs.f64 %fd{{[0-9]+}}, %fd{{[0-9]+}};
; CHECK: ret;
	%x = call double @llvm.fabs.f64(double %d)
	ret double %x
}

define float @test_nvvm_sqrt(float %a) {
; CHECK: sqrt.rn.f32 %f{{[0-9]+}}, %f{{[0-9]+}};
; CHECK: ret;
  %val = call float @llvm.nvvm.sqrt.f(float %a)
  ret float %val
}


declare float @llvm.fabs.f32(float)
declare double @llvm.fabs.f64(double)
declare float @llvm.nvvm.sqrt.f(float)
