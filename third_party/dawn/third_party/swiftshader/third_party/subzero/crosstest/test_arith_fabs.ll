declare float @llvm.fabs.f32(float)
declare double @llvm.fabs.f64(double)
declare <4 x float> @llvm.fabs.v4f32(<4 x float>)

define float @_Z6myFabsf(float %a) {
  %x = call float @llvm.fabs.f32(float %a)
  ret float %x
}

define double @_Z6myFabsd(double %a) {
  %x = call double @llvm.fabs.f64(double %a)
  ret double %x
}

define <4 x float> @_Z6myFabsDv4_f(<4 x float> %a) {
  %x = call <4 x float> @llvm.fabs.v4f32(<4 x float> %a)
  ret <4 x float> %x
}
