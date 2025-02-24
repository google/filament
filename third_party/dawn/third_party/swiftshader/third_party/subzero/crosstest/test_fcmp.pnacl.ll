; This file is extracted from fp.pnacl.ll and vector-fcmp.ll in the lit
; tests, with the "internal" attribute removed from the functions.

define i32 @fcmpFalseFloat(float %a, float %b) {
entry:
  %cmp = fcmp false float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpFalseFloat:
; CHECK: mov {{.*}}, 0

define i32 @fcmpFalseDouble(double %a, double %b) {
entry:
  %cmp = fcmp false double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpFalseDouble:
; CHECK: mov {{.*}}, 0

define i32 @fcmpOeqFloat(float %a, float %b) {
entry:
  %cmp = fcmp oeq float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOeqFloat:
; CHECK: ucomiss
; CHECK: jne .
; CHECK: jp .

define i32 @fcmpOeqDouble(double %a, double %b) {
entry:
  %cmp = fcmp oeq double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOeqDouble:
; CHECK: ucomisd
; CHECK: jne .
; CHECK: jp .

define i32 @fcmpOgtFloat(float %a, float %b) {
entry:
  %cmp = fcmp ogt float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOgtFloat:
; CHECK: ucomiss
; CHECK: ja .

define i32 @fcmpOgtDouble(double %a, double %b) {
entry:
  %cmp = fcmp ogt double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOgtDouble:
; CHECK: ucomisd
; CHECK: ja .

define i32 @fcmpOgeFloat(float %a, float %b) {
entry:
  %cmp = fcmp oge float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOgeFloat:
; CHECK: ucomiss
; CHECK: jae .

define i32 @fcmpOgeDouble(double %a, double %b) {
entry:
  %cmp = fcmp oge double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOgeDouble:
; CHECK: ucomisd
; CHECK: jae .

define i32 @fcmpOltFloat(float %a, float %b) {
entry:
  %cmp = fcmp olt float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOltFloat:
; CHECK: ucomiss
; CHECK: ja .

define i32 @fcmpOltDouble(double %a, double %b) {
entry:
  %cmp = fcmp olt double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOltDouble:
; CHECK: ucomisd
; CHECK: ja .

define i32 @fcmpOleFloat(float %a, float %b) {
entry:
  %cmp = fcmp ole float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOleFloat:
; CHECK: ucomiss
; CHECK: jae .

define i32 @fcmpOleDouble(double %a, double %b) {
entry:
  %cmp = fcmp ole double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOleDouble:
; CHECK: ucomisd
; CHECK: jae .

define i32 @fcmpOneFloat(float %a, float %b) {
entry:
  %cmp = fcmp one float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOneFloat:
; CHECK: ucomiss
; CHECK: jne .

define i32 @fcmpOneDouble(double %a, double %b) {
entry:
  %cmp = fcmp one double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOneDouble:
; CHECK: ucomisd
; CHECK: jne .

define i32 @fcmpOrdFloat(float %a, float %b) {
entry:
  %cmp = fcmp ord float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOrdFloat:
; CHECK: ucomiss
; CHECK: jnp .

define i32 @fcmpOrdDouble(double %a, double %b) {
entry:
  %cmp = fcmp ord double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOrdDouble:
; CHECK: ucomisd
; CHECK: jnp .

define i32 @fcmpUeqFloat(float %a, float %b) {
entry:
  %cmp = fcmp ueq float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUeqFloat:
; CHECK: ucomiss
; CHECK: je .

define i32 @fcmpUeqDouble(double %a, double %b) {
entry:
  %cmp = fcmp ueq double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUeqDouble:
; CHECK: ucomisd
; CHECK: je .

define i32 @fcmpUgtFloat(float %a, float %b) {
entry:
  %cmp = fcmp ugt float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUgtFloat:
; CHECK: ucomiss
; CHECK: jb .

define i32 @fcmpUgtDouble(double %a, double %b) {
entry:
  %cmp = fcmp ugt double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUgtDouble:
; CHECK: ucomisd
; CHECK: jb .

define i32 @fcmpUgeFloat(float %a, float %b) {
entry:
  %cmp = fcmp uge float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUgeFloat:
; CHECK: ucomiss
; CHECK: jbe .

define i32 @fcmpUgeDouble(double %a, double %b) {
entry:
  %cmp = fcmp uge double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUgeDouble:
; CHECK: ucomisd
; CHECK: jbe .

define i32 @fcmpUltFloat(float %a, float %b) {
entry:
  %cmp = fcmp ult float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUltFloat:
; CHECK: ucomiss
; CHECK: jb .

define i32 @fcmpUltDouble(double %a, double %b) {
entry:
  %cmp = fcmp ult double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUltDouble:
; CHECK: ucomisd
; CHECK: jb .

define i32 @fcmpUleFloat(float %a, float %b) {
entry:
  %cmp = fcmp ule float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUleFloat:
; CHECK: ucomiss
; CHECK: jbe .

define i32 @fcmpUleDouble(double %a, double %b) {
entry:
  %cmp = fcmp ule double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUleDouble:
; CHECK: ucomisd
; CHECK: jbe .

define i32 @fcmpUneFloat(float %a, float %b) {
entry:
  %cmp = fcmp une float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUneFloat:
; CHECK: ucomiss
; CHECK: je .
; CHECK: jnp .

define i32 @fcmpUneDouble(double %a, double %b) {
entry:
  %cmp = fcmp une double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUneDouble:
; CHECK: ucomisd
; CHECK: je .
; CHECK: jnp .

define i32 @fcmpUnoFloat(float %a, float %b) {
entry:
  %cmp = fcmp uno float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUnoFloat:
; CHECK: ucomiss
; CHECK: jp .

define i32 @fcmpUnoDouble(double %a, double %b) {
entry:
  %cmp = fcmp uno double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUnoDouble:
; CHECK: ucomisd
; CHECK: jp .

define i32 @fcmpTrueFloat(float %a, float %b) {
entry:
  %cmp = fcmp true float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpTrueFloat:
; CHECK: mov {{.*}}, 1

define i32 @fcmpTrueDouble(double %a, double %b) {
entry:
  %cmp = fcmp true double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpTrueDouble:
; CHECK: mov {{.*}}, 1

define i32 @fcmpSelectFalseFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp false float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectFalseFloat:
; CHECK: mov {{.*}}, 0

define i32 @fcmpSelectFalseDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp false double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectFalseDouble:
; CHECK: mov {{.*}}, 0

define i32 @fcmpSelectOeqFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp oeq float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOeqFloat:
; CHECK: ucomiss
; CHECK: jne .
; CHECK: jp .

define i32 @fcmpSelectOeqDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp oeq double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOeqDouble:
; CHECK: ucomisd
; CHECK: jne .
; CHECK: jp .

define i32 @fcmpSelectOgtFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ogt float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOgtFloat:
; CHECK: ucomiss
; CHECK: ja .

define i32 @fcmpSelectOgtDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ogt double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOgtDouble:
; CHECK: ucomisd
; CHECK: ja .

define i32 @fcmpSelectOgeFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp oge float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOgeFloat:
; CHECK: ucomiss
; CHECK: jae .

define i32 @fcmpSelectOgeDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp oge double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOgeDouble:
; CHECK: ucomisd
; CHECK: jae .

define i32 @fcmpSelectOltFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp olt float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOltFloat:
; CHECK: ucomiss
; CHECK: ja .

define i32 @fcmpSelectOltDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp olt double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOltDouble:
; CHECK: ucomisd
; CHECK: ja .

define i32 @fcmpSelectOleFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ole float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOleFloat:
; CHECK: ucomiss
; CHECK: jae .

define i32 @fcmpSelectOleDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ole double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOleDouble:
; CHECK: ucomisd
; CHECK: jae .

define i32 @fcmpSelectOneFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp one float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOneFloat:
; CHECK: ucomiss
; CHECK: jne .

define i32 @fcmpSelectOneDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp one double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOneDouble:
; CHECK: ucomisd
; CHECK: jne .

define i32 @fcmpSelectOrdFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ord float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOrdFloat:
; CHECK: ucomiss
; CHECK: jnp .

define i32 @fcmpSelectOrdDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ord double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectOrdDouble:
; CHECK: ucomisd
; CHECK: jnp .

define i32 @fcmpSelectUeqFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ueq float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUeqFloat:
; CHECK: ucomiss
; CHECK: je .

define i32 @fcmpSelectUeqDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ueq double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUeqDouble:
; CHECK: ucomisd
; CHECK: je .

define i32 @fcmpSelectUgtFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ugt float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUgtFloat:
; CHECK: ucomiss
; CHECK: jb .

define i32 @fcmpSelectUgtDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ugt double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUgtDouble:
; CHECK: ucomisd
; CHECK: jb .

define i32 @fcmpSelectUgeFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp uge float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUgeFloat:
; CHECK: ucomiss
; CHECK: jbe .

define i32 @fcmpSelectUgeDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp uge double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUgeDouble:
; CHECK: ucomisd
; CHECK: jbe .

define i32 @fcmpSelectUltFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ult float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUltFloat:
; CHECK: ucomiss
; CHECK: jb .

define i32 @fcmpSelectUltDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ult double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUltDouble:
; CHECK: ucomisd
; CHECK: jb .

define i32 @fcmpSelectUleFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ule float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUleFloat:
; CHECK: ucomiss
; CHECK: jbe .

define i32 @fcmpSelectUleDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp ule double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUleDouble:
; CHECK: ucomisd
; CHECK: jbe .

define i32 @fcmpSelectUneFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp une float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUneFloat:
; CHECK: ucomiss
; CHECK: je .
; CHECK: jnp .

define i32 @fcmpSelectUneDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp une double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUneDouble:
; CHECK: ucomisd
; CHECK: je .
; CHECK: jnp .

define i32 @fcmpSelectUnoFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp uno float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUnoFloat:
; CHECK: ucomiss
; CHECK: jp .

define i32 @fcmpSelectUnoDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp uno double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectUnoDouble:
; CHECK: ucomisd
; CHECK: jp .

define i32 @fcmpSelectTrueFloat(float %a, float %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp true float %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectTrueFloat:
; CHECK: mov {{.*}}, 1

define i32 @fcmpSelectTrueDouble(double %a, double %b, i32 %c, i32 %d) {
entry:
  %cmp = fcmp true double %a, %b
  %cmp.ret_ext = select i1 %cmp, i32 %c, i32 %d
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpSelectTrueDouble:
; CHECK: mov {{.*}}, 1

define <4 x i32> @fcmpFalseVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp false <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpFalseVector:
; CHECK: pxor
}

define <4 x i32> @fcmpOeqVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp oeq <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpOeqVector:
; CHECK: cmpeqps
}

define <4 x i32> @fcmpOgeVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp oge <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpOgeVector:
; CHECK: cmpleps
}

define <4 x i32> @fcmpOgtVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp ogt <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpOgtVector:
; CHECK: cmpltps
}

define <4 x i32> @fcmpOleVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp ole <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpOleVector:
; CHECK: cmpleps
}

define <4 x i32> @fcmpOltVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp olt <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpOltVector:
; CHECK: cmpltps
}

define <4 x i32> @fcmpOneVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp one <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpOneVector:
; CHECK: cmpneqps
; CHECK: cmpordps
; CHECK: pand
}

define <4 x i32> @fcmpOrdVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp ord <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpOrdVector:
; CHECK: cmpordps
}

define <4 x i32> @fcmpTrueVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp true <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpTrueVector:
; CHECK: pcmpeqd
}

define <4 x i32> @fcmpUeqVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp ueq <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpUeqVector:
; CHECK: cmpeqps
; CHECK: cmpunordps
; CHECK: por
}

define <4 x i32> @fcmpUgeVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp uge <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpUgeVector:
; CHECK: cmpnltps
}

define <4 x i32> @fcmpUgtVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp ugt <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpUgtVector:
; CHECK: cmpnleps
}

define <4 x i32> @fcmpUleVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp ule <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpUleVector:
; CHECK: cmpnltps
}

define <4 x i32> @fcmpUltVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp ult <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpUltVector:
; CHECK: cmpnleps
}

define <4 x i32> @fcmpUneVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp une <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpUneVector:
; CHECK: cmpneqps
}

define <4 x i32> @fcmpUnoVector(<4 x float> %a, <4 x float> %b) {
entry:
  %res.trunc = fcmp uno <4 x float> %a, %b
  %res = sext <4 x i1> %res.trunc to <4 x i32>
  ret <4 x i32> %res
; CHECK-LABEL: fcmpUnoVector:
; CHECK: cmpunordps
}
