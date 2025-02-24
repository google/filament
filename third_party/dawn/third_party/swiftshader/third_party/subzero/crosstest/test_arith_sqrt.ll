declare float @llvm.sqrt.f32(float)
declare double @llvm.sqrt.f64(double)

define float @_Z6mySqrtf(float %a) {
  %x = call float @llvm.sqrt.f32(float %a)
  ret float %x
}

define double @_Z6mySqrtd(double %a) {
  %x = call double @llvm.sqrt.f64(double %a)
  ret double %x
}
