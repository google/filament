#version 460
precision highp float;
precision highp int;
out float o1;
void main()
{
	const mat2 cval = matrixCompMult(mat2(1.0), mat2(1.0));
	const mat3 cval1 = matrixCompMult(mat3(1.0), mat3(1.0));
	const mat4 cval2 = matrixCompMult(mat4(1.0), mat4(1.0));
	const mat2x3 cval3 = matrixCompMult(mat2x3(1.0), mat2x3(1.0));
	const mat2x4 cval4 = matrixCompMult(mat2x4(1.0), mat2x4(1.0));
	const mat3x2 cval5 = matrixCompMult(mat3x2(1.0), mat3x2(1.0));
	const mat3x4 cval6 = matrixCompMult(mat3x4(1.0), mat3x4(1.0));
	const mat4x2 cval7 = matrixCompMult(mat4x2(1.0), mat4x2(1.0));
    const mat4x3 cval8 = matrixCompMult(mat4x3(1.0), mat4x3(1.0));
	const dmat2 cval9 = matrixCompMult(dmat2(1.0), dmat2(1.0));
	const dmat3 cval10 = matrixCompMult(dmat3(1.0), dmat3(1.0));
	const dmat4 cval11 = matrixCompMult(dmat4(1.0), dmat4(1.0));
	const dmat2x3 cval12 = matrixCompMult(dmat2x3(1.0), dmat2x3(1.0));
	const dmat2x4 cval13 = matrixCompMult(dmat2x4(1.0), dmat2x4(1.0));
	const dmat3x2 cval14 = matrixCompMult(dmat3x2(1.0), dmat3x2(1.0));
	const dmat3x4 cval15 = matrixCompMult(dmat3x4(1.0), dmat3x4(1.0));
	const dmat4x2 cval16 = matrixCompMult(dmat4x2(1.0), dmat4x2(1.0));
    const dmat4x3 cval17 = matrixCompMult(dmat4x3(1.0), dmat4x3(1.0));
	
	o1 = float(cval[0][0] + cval1[0][0] + cval2[0][0] + cval3[0][0] + 
	     cval4[0][0] + cval5[0][0] + cval6[0][0] + cval7[0][0] + 
		 cval8[0][0] + cval9[0][0] + cval10[0][0] + cval11[0][0] + 
		 cval12[0][0] + cval13[0][0] + cval14[0][0] + cval15[0][0] + 
		 cval16[0][0] + cval17[0][0]);
}

