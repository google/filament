#version 310 es
precision mediump float;

layout(location = 0) out vec4 FragColor;
layout(constant_id = 1) const float a = 1.5;
layout(constant_id = 2) const float b = 2.5;
layout(constant_id = 3) const int c = 3;
layout(constant_id = 4) const int d = 4;
layout(constant_id = 5) const uint e = 5u;
layout(constant_id = 6) const uint f = 6u;
layout(constant_id = 7) const bool g = false;
layout(constant_id = 8) const bool h = true;

// glslang doesn't seem to support partial spec constants or composites yet, so only test the basics.

struct Foo
{
	float elems[d + 2];
};

void main()
{
	float t0 = a;
	float t1 = b;

	uint c0 = uint(c); // OpIAdd with different types.
	// FConvert, float-to-double.
	int c1 = -c; // SNegate
	int c2 = ~c; // OpNot
	int c3 = c + d; // OpIAdd
	int c4 = c - d; // OpISub
	int c5 = c * d; // OpIMul
	int c6 = c / d; // OpSDiv
	uint c7 = e / f; // OpUDiv
	int c8 = c % d; // OpSMod
	uint c9 = e % f; // OpUMod
	// TODO: OpSRem, any way to access this in GLSL?
	int c10 = c >> d; // OpShiftRightArithmetic
	uint c11 = e >> f; // OpShiftRightLogical
	int c12 = c << d; // OpShiftLeftLogical
	int c13 = c | d; // OpBitwiseOr
	int c14 = c ^ d; // OpBitwiseXor
	int c15 = c & d; // OpBitwiseAnd
	// VectorShuffle, CompositeExtract, CompositeInsert, not testable atm.
	bool c16 = g || h; // OpLogicalOr
	bool c17 = g && h; // OpLogicalAnd
	bool c18 = !g; // OpLogicalNot
	bool c19 = g == h; // OpLogicalEqual
	bool c20 = g != h; // OpLogicalNotEqual
	// OpSelect not testable atm.
	bool c21 = c == d; // OpIEqual
	bool c22 = c != d; // OpINotEqual
	bool c23 = c < d; // OpSLessThan
	bool c24 = e < f; // OpULessThan
	bool c25 = c > d; // OpSGreaterThan
	bool c26 = e > f; // OpUGreaterThan
	bool c27 = c <= d; // OpSLessThanEqual
	bool c28 = e <= f; // OpULessThanEqual
	bool c29 = c >= d; // OpSGreaterThanEqual
	bool c30 = e >= f; // OpUGreaterThanEqual
	// OpQuantizeToF16 not testable atm.

	int c31 = c8 + c3;

	int c32 = int(e); // OpIAdd with different types.
	bool c33 = bool(c); // int -> bool
	bool c34 = bool(e); // uint -> bool
	int c35 = int(g); // bool -> int
	uint c36 = uint(g); // bool -> uint
	float c37 = float(g); // bool -> float

	// Flexible sized arrays with spec constants and spec constant ops.
	float vec0[c + 3][8];
	float vec1[c + 2];

	Foo foo;
	FragColor = vec4(t0 + t1) + vec0[0][0] + vec1[0] + foo.elems[c];
}
