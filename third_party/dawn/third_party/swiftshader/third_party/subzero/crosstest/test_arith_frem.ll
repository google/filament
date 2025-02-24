define float @_Z6myFremff(float %a, float %b) {
  %rem = frem float %a, %b
  ret float %rem
}

define double @_Z6myFremdd(double %a, double %b) {
  %rem = frem double %a, %b
  ret double %rem
}

define <4 x float> @_Z6myFremDv4_fS_(<4 x float> %a, <4 x float> %b) {
  %rem = frem <4 x float> %a, %b
  ret <4 x float> %rem
}
