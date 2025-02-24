// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

namespace rr {

template<class T>
struct Scalar;

template<class Vector4>
struct XYZW;

template<class T>
class RValue;

template<class Vector4, int T>
class Swizzle2
{
	friend Vector4;

public:
	operator RValue<Vector4>() const;

private:
	Vector4 *parent;
};

template<class Vector4, int T>
class Swizzle4
{
public:
	operator RValue<Vector4>() const;

private:
	Vector4 *parent;
};

template<class Vector4, int T>
class SwizzleMask4
{
	friend XYZW<Vector4>;

public:
	operator RValue<Vector4>() const;

	RValue<Vector4> operator=(RValue<Vector4> rhs);
	RValue<Vector4> operator=(RValue<typename Scalar<Vector4>::Type> rhs);

private:
	Vector4 *parent;
};

template<class Vector4, int T>
class SwizzleMask1
{
public:
	operator RValue<typename Scalar<Vector4>::Type>() const;
	operator RValue<Vector4>() const;

	RValue<Vector4> operator=(float x);
	RValue<Vector4> operator=(RValue<Vector4> rhs);
	RValue<Vector4> operator=(RValue<typename Scalar<Vector4>::Type> rhs);

private:
	Vector4 *parent;
};

template<class Vector4, int T>
RValue<typename Scalar<Vector4>::Type> operator+(RValue<typename Scalar<Vector4>::Type> lhs, SwizzleMask1<Vector4, T> rhs)
{
	return lhs + RValue<typename Scalar<Vector4>::Type>(rhs);
}

template<class Vector4, int T>
RValue<typename Scalar<Vector4>::Type> operator+(SwizzleMask1<Vector4, T> lhs, RValue<typename Scalar<Vector4>::Type> rhs)
{
	return RValue<typename Scalar<Vector4>::Type>(lhs) + rhs;
}

template<class Vector4, int T>
RValue<typename Scalar<Vector4>::Type> operator-(RValue<typename Scalar<Vector4>::Type> lhs, SwizzleMask1<Vector4, T> rhs)
{
	return lhs - RValue<typename Scalar<Vector4>::Type>(rhs);
}

template<class Vector4, int T>
RValue<typename Scalar<Vector4>::Type> operator-(SwizzleMask1<Vector4, T> lhs, RValue<typename Scalar<Vector4>::Type> rhs)
{
	return RValue<typename Scalar<Vector4>::Type>(lhs) - rhs;
}

template<class Vector4, int T>
class SwizzleMask2
{
	friend Vector4;

public:
	operator RValue<Vector4>() const;

	RValue<Vector4> operator=(RValue<Vector4> rhs);

private:
	Vector4 *parent;
};

template<class Vector4>
struct XYZW
{
	friend Vector4;

private:
	XYZW(Vector4 *parent)
	{
		xyzw.parent = parent;
	}

public:
	union
	{
		SwizzleMask1<Vector4, 0x0000> x;
		SwizzleMask1<Vector4, 0x1111> y;
		SwizzleMask1<Vector4, 0x2222> z;
		SwizzleMask1<Vector4, 0x3333> w;
		Swizzle2<Vector4, 0x0000> xx;
		Swizzle2<Vector4, 0x1000> yx;
		Swizzle2<Vector4, 0x2000> zx;
		Swizzle2<Vector4, 0x3000> wx;
		SwizzleMask2<Vector4, 0x0111> xy;
		Swizzle2<Vector4, 0x1111> yy;
		Swizzle2<Vector4, 0x2111> zy;
		Swizzle2<Vector4, 0x3111> wy;
		SwizzleMask2<Vector4, 0x0222> xz;
		SwizzleMask2<Vector4, 0x1222> yz;
		Swizzle2<Vector4, 0x2222> zz;
		Swizzle2<Vector4, 0x3222> wz;
		SwizzleMask2<Vector4, 0x0333> xw;
		SwizzleMask2<Vector4, 0x1333> yw;
		SwizzleMask2<Vector4, 0x2333> zw;
		Swizzle2<Vector4, 0x3333> ww;
		Swizzle4<Vector4, 0x0000> xxx;
		Swizzle4<Vector4, 0x1000> yxx;
		Swizzle4<Vector4, 0x2000> zxx;
		Swizzle4<Vector4, 0x3000> wxx;
		Swizzle4<Vector4, 0x0100> xyx;
		Swizzle4<Vector4, 0x1100> yyx;
		Swizzle4<Vector4, 0x2100> zyx;
		Swizzle4<Vector4, 0x3100> wyx;
		Swizzle4<Vector4, 0x0200> xzx;
		Swizzle4<Vector4, 0x1200> yzx;
		Swizzle4<Vector4, 0x2200> zzx;
		Swizzle4<Vector4, 0x3200> wzx;
		Swizzle4<Vector4, 0x0300> xwx;
		Swizzle4<Vector4, 0x1300> ywx;
		Swizzle4<Vector4, 0x2300> zwx;
		Swizzle4<Vector4, 0x3300> wwx;
		Swizzle4<Vector4, 0x0011> xxy;
		Swizzle4<Vector4, 0x1011> yxy;
		Swizzle4<Vector4, 0x2011> zxy;
		Swizzle4<Vector4, 0x3011> wxy;
		Swizzle4<Vector4, 0x0111> xyy;
		Swizzle4<Vector4, 0x1111> yyy;
		Swizzle4<Vector4, 0x2111> zyy;
		Swizzle4<Vector4, 0x3111> wyy;
		Swizzle4<Vector4, 0x0211> xzy;
		Swizzle4<Vector4, 0x1211> yzy;
		Swizzle4<Vector4, 0x2211> zzy;
		Swizzle4<Vector4, 0x3211> wzy;
		Swizzle4<Vector4, 0x0311> xwy;
		Swizzle4<Vector4, 0x1311> ywy;
		Swizzle4<Vector4, 0x2311> zwy;
		Swizzle4<Vector4, 0x3311> wwy;
		Swizzle4<Vector4, 0x0022> xxz;
		Swizzle4<Vector4, 0x1022> yxz;
		Swizzle4<Vector4, 0x2022> zxz;
		Swizzle4<Vector4, 0x3022> wxz;
		SwizzleMask4<Vector4, 0x0122> xyz;
		Swizzle4<Vector4, 0x1122> yyz;
		Swizzle4<Vector4, 0x2122> zyz;
		Swizzle4<Vector4, 0x3122> wyz;
		Swizzle4<Vector4, 0x0222> xzz;
		Swizzle4<Vector4, 0x1222> yzz;
		Swizzle4<Vector4, 0x2222> zzz;
		Swizzle4<Vector4, 0x3222> wzz;
		Swizzle4<Vector4, 0x0322> xwz;
		Swizzle4<Vector4, 0x1322> ywz;
		Swizzle4<Vector4, 0x2322> zwz;
		Swizzle4<Vector4, 0x3322> wwz;
		Swizzle4<Vector4, 0x0033> xxw;
		Swizzle4<Vector4, 0x1033> yxw;
		Swizzle4<Vector4, 0x2033> zxw;
		Swizzle4<Vector4, 0x3033> wxw;
		SwizzleMask4<Vector4, 0x0133> xyw;
		Swizzle4<Vector4, 0x1133> yyw;
		Swizzle4<Vector4, 0x2133> zyw;
		Swizzle4<Vector4, 0x3133> wyw;
		SwizzleMask4<Vector4, 0x0233> xzw;
		SwizzleMask4<Vector4, 0x1233> yzw;
		Swizzle4<Vector4, 0x2233> zzw;
		Swizzle4<Vector4, 0x3233> wzw;
		Swizzle4<Vector4, 0x0333> xww;
		Swizzle4<Vector4, 0x1333> yww;
		Swizzle4<Vector4, 0x2333> zww;
		Swizzle4<Vector4, 0x3333> www;
		Swizzle4<Vector4, 0x0000> xxxx;
		Swizzle4<Vector4, 0x1000> yxxx;
		Swizzle4<Vector4, 0x2000> zxxx;
		Swizzle4<Vector4, 0x3000> wxxx;
		Swizzle4<Vector4, 0x0100> xyxx;
		Swizzle4<Vector4, 0x1100> yyxx;
		Swizzle4<Vector4, 0x2100> zyxx;
		Swizzle4<Vector4, 0x3100> wyxx;
		Swizzle4<Vector4, 0x0200> xzxx;
		Swizzle4<Vector4, 0x1200> yzxx;
		Swizzle4<Vector4, 0x2200> zzxx;
		Swizzle4<Vector4, 0x3200> wzxx;
		Swizzle4<Vector4, 0x0300> xwxx;
		Swizzle4<Vector4, 0x1300> ywxx;
		Swizzle4<Vector4, 0x2300> zwxx;
		Swizzle4<Vector4, 0x3300> wwxx;
		Swizzle4<Vector4, 0x0010> xxyx;
		Swizzle4<Vector4, 0x1010> yxyx;
		Swizzle4<Vector4, 0x2010> zxyx;
		Swizzle4<Vector4, 0x3010> wxyx;
		Swizzle4<Vector4, 0x0110> xyyx;
		Swizzle4<Vector4, 0x1110> yyyx;
		Swizzle4<Vector4, 0x2110> zyyx;
		Swizzle4<Vector4, 0x3110> wyyx;
		Swizzle4<Vector4, 0x0210> xzyx;
		Swizzle4<Vector4, 0x1210> yzyx;
		Swizzle4<Vector4, 0x2210> zzyx;
		Swizzle4<Vector4, 0x3210> wzyx;
		Swizzle4<Vector4, 0x0310> xwyx;
		Swizzle4<Vector4, 0x1310> ywyx;
		Swizzle4<Vector4, 0x2310> zwyx;
		Swizzle4<Vector4, 0x3310> wwyx;
		Swizzle4<Vector4, 0x0020> xxzx;
		Swizzle4<Vector4, 0x1020> yxzx;
		Swizzle4<Vector4, 0x2020> zxzx;
		Swizzle4<Vector4, 0x3020> wxzx;
		Swizzle4<Vector4, 0x0120> xyzx;
		Swizzle4<Vector4, 0x1120> yyzx;
		Swizzle4<Vector4, 0x2120> zyzx;
		Swizzle4<Vector4, 0x3120> wyzx;
		Swizzle4<Vector4, 0x0220> xzzx;
		Swizzle4<Vector4, 0x1220> yzzx;
		Swizzle4<Vector4, 0x2220> zzzx;
		Swizzle4<Vector4, 0x3220> wzzx;
		Swizzle4<Vector4, 0x0320> xwzx;
		Swizzle4<Vector4, 0x1320> ywzx;
		Swizzle4<Vector4, 0x2320> zwzx;
		Swizzle4<Vector4, 0x3320> wwzx;
		Swizzle4<Vector4, 0x0030> xxwx;
		Swizzle4<Vector4, 0x1030> yxwx;
		Swizzle4<Vector4, 0x2030> zxwx;
		Swizzle4<Vector4, 0x3030> wxwx;
		Swizzle4<Vector4, 0x0130> xywx;
		Swizzle4<Vector4, 0x1130> yywx;
		Swizzle4<Vector4, 0x2130> zywx;
		Swizzle4<Vector4, 0x3130> wywx;
		Swizzle4<Vector4, 0x0230> xzwx;
		Swizzle4<Vector4, 0x1230> yzwx;
		Swizzle4<Vector4, 0x2230> zzwx;
		Swizzle4<Vector4, 0x3230> wzwx;
		Swizzle4<Vector4, 0x0330> xwwx;
		Swizzle4<Vector4, 0x1330> ywwx;
		Swizzle4<Vector4, 0x2330> zwwx;
		Swizzle4<Vector4, 0x3330> wwwx;
		Swizzle4<Vector4, 0x0001> xxxy;
		Swizzle4<Vector4, 0x1001> yxxy;
		Swizzle4<Vector4, 0x2001> zxxy;
		Swizzle4<Vector4, 0x3001> wxxy;
		Swizzle4<Vector4, 0x0101> xyxy;
		Swizzle4<Vector4, 0x1101> yyxy;
		Swizzle4<Vector4, 0x2101> zyxy;
		Swizzle4<Vector4, 0x3101> wyxy;
		Swizzle4<Vector4, 0x0201> xzxy;
		Swizzle4<Vector4, 0x1201> yzxy;
		Swizzle4<Vector4, 0x2201> zzxy;
		Swizzle4<Vector4, 0x3201> wzxy;
		Swizzle4<Vector4, 0x0301> xwxy;
		Swizzle4<Vector4, 0x1301> ywxy;
		Swizzle4<Vector4, 0x2301> zwxy;
		Swizzle4<Vector4, 0x3301> wwxy;
		Swizzle4<Vector4, 0x0011> xxyy;
		Swizzle4<Vector4, 0x1011> yxyy;
		Swizzle4<Vector4, 0x2011> zxyy;
		Swizzle4<Vector4, 0x3011> wxyy;
		Swizzle4<Vector4, 0x0111> xyyy;
		Swizzle4<Vector4, 0x1111> yyyy;
		Swizzle4<Vector4, 0x2111> zyyy;
		Swizzle4<Vector4, 0x3111> wyyy;
		Swizzle4<Vector4, 0x0211> xzyy;
		Swizzle4<Vector4, 0x1211> yzyy;
		Swizzle4<Vector4, 0x2211> zzyy;
		Swizzle4<Vector4, 0x3211> wzyy;
		Swizzle4<Vector4, 0x0311> xwyy;
		Swizzle4<Vector4, 0x1311> ywyy;
		Swizzle4<Vector4, 0x2311> zwyy;
		Swizzle4<Vector4, 0x3311> wwyy;
		Swizzle4<Vector4, 0x0021> xxzy;
		Swizzle4<Vector4, 0x1021> yxzy;
		Swizzle4<Vector4, 0x2021> zxzy;
		Swizzle4<Vector4, 0x3021> wxzy;
		Swizzle4<Vector4, 0x0121> xyzy;
		Swizzle4<Vector4, 0x1121> yyzy;
		Swizzle4<Vector4, 0x2121> zyzy;
		Swizzle4<Vector4, 0x3121> wyzy;
		Swizzle4<Vector4, 0x0221> xzzy;
		Swizzle4<Vector4, 0x1221> yzzy;
		Swizzle4<Vector4, 0x2221> zzzy;
		Swizzle4<Vector4, 0x3221> wzzy;
		Swizzle4<Vector4, 0x0321> xwzy;
		Swizzle4<Vector4, 0x1321> ywzy;
		Swizzle4<Vector4, 0x2321> zwzy;
		Swizzle4<Vector4, 0x3321> wwzy;
		Swizzle4<Vector4, 0x0031> xxwy;
		Swizzle4<Vector4, 0x1031> yxwy;
		Swizzle4<Vector4, 0x2031> zxwy;
		Swizzle4<Vector4, 0x3031> wxwy;
		Swizzle4<Vector4, 0x0131> xywy;
		Swizzle4<Vector4, 0x1131> yywy;
		Swizzle4<Vector4, 0x2131> zywy;
		Swizzle4<Vector4, 0x3131> wywy;
		Swizzle4<Vector4, 0x0231> xzwy;
		Swizzle4<Vector4, 0x1231> yzwy;
		Swizzle4<Vector4, 0x2231> zzwy;
		Swizzle4<Vector4, 0x3231> wzwy;
		Swizzle4<Vector4, 0x0331> xwwy;
		Swizzle4<Vector4, 0x1331> ywwy;
		Swizzle4<Vector4, 0x2331> zwwy;
		Swizzle4<Vector4, 0x3331> wwwy;
		Swizzle4<Vector4, 0x0002> xxxz;
		Swizzle4<Vector4, 0x1002> yxxz;
		Swizzle4<Vector4, 0x2002> zxxz;
		Swizzle4<Vector4, 0x3002> wxxz;
		Swizzle4<Vector4, 0x0102> xyxz;
		Swizzle4<Vector4, 0x1102> yyxz;
		Swizzle4<Vector4, 0x2102> zyxz;
		Swizzle4<Vector4, 0x3102> wyxz;
		Swizzle4<Vector4, 0x0202> xzxz;
		Swizzle4<Vector4, 0x1202> yzxz;
		Swizzle4<Vector4, 0x2202> zzxz;
		Swizzle4<Vector4, 0x3202> wzxz;
		Swizzle4<Vector4, 0x0302> xwxz;
		Swizzle4<Vector4, 0x1302> ywxz;
		Swizzle4<Vector4, 0x2302> zwxz;
		Swizzle4<Vector4, 0x3302> wwxz;
		Swizzle4<Vector4, 0x0012> xxyz;
		Swizzle4<Vector4, 0x1012> yxyz;
		Swizzle4<Vector4, 0x2012> zxyz;
		Swizzle4<Vector4, 0x3012> wxyz;
		Swizzle4<Vector4, 0x0112> xyyz;
		Swizzle4<Vector4, 0x1112> yyyz;
		Swizzle4<Vector4, 0x2112> zyyz;
		Swizzle4<Vector4, 0x3112> wyyz;
		Swizzle4<Vector4, 0x0212> xzyz;
		Swizzle4<Vector4, 0x1212> yzyz;
		Swizzle4<Vector4, 0x2212> zzyz;
		Swizzle4<Vector4, 0x3212> wzyz;
		Swizzle4<Vector4, 0x0312> xwyz;
		Swizzle4<Vector4, 0x1312> ywyz;
		Swizzle4<Vector4, 0x2312> zwyz;
		Swizzle4<Vector4, 0x3312> wwyz;
		Swizzle4<Vector4, 0x0022> xxzz;
		Swizzle4<Vector4, 0x1022> yxzz;
		Swizzle4<Vector4, 0x2022> zxzz;
		Swizzle4<Vector4, 0x3022> wxzz;
		Swizzle4<Vector4, 0x0122> xyzz;
		Swizzle4<Vector4, 0x1122> yyzz;
		Swizzle4<Vector4, 0x2122> zyzz;
		Swizzle4<Vector4, 0x3122> wyzz;
		Swizzle4<Vector4, 0x0222> xzzz;
		Swizzle4<Vector4, 0x1222> yzzz;
		Swizzle4<Vector4, 0x2222> zzzz;
		Swizzle4<Vector4, 0x3222> wzzz;
		Swizzle4<Vector4, 0x0322> xwzz;
		Swizzle4<Vector4, 0x1322> ywzz;
		Swizzle4<Vector4, 0x2322> zwzz;
		Swizzle4<Vector4, 0x3322> wwzz;
		Swizzle4<Vector4, 0x0032> xxwz;
		Swizzle4<Vector4, 0x1032> yxwz;
		Swizzle4<Vector4, 0x2032> zxwz;
		Swizzle4<Vector4, 0x3032> wxwz;
		Swizzle4<Vector4, 0x0132> xywz;
		Swizzle4<Vector4, 0x1132> yywz;
		Swizzle4<Vector4, 0x2132> zywz;
		Swizzle4<Vector4, 0x3132> wywz;
		Swizzle4<Vector4, 0x0232> xzwz;
		Swizzle4<Vector4, 0x1232> yzwz;
		Swizzle4<Vector4, 0x2232> zzwz;
		Swizzle4<Vector4, 0x3232> wzwz;
		Swizzle4<Vector4, 0x0332> xwwz;
		Swizzle4<Vector4, 0x1332> ywwz;
		Swizzle4<Vector4, 0x2332> zwwz;
		Swizzle4<Vector4, 0x3332> wwwz;
		Swizzle4<Vector4, 0x0003> xxxw;
		Swizzle4<Vector4, 0x1003> yxxw;
		Swizzle4<Vector4, 0x2003> zxxw;
		Swizzle4<Vector4, 0x3003> wxxw;
		Swizzle4<Vector4, 0x0103> xyxw;
		Swizzle4<Vector4, 0x1103> yyxw;
		Swizzle4<Vector4, 0x2103> zyxw;
		Swizzle4<Vector4, 0x3103> wyxw;
		Swizzle4<Vector4, 0x0203> xzxw;
		Swizzle4<Vector4, 0x1203> yzxw;
		Swizzle4<Vector4, 0x2203> zzxw;
		Swizzle4<Vector4, 0x3203> wzxw;
		Swizzle4<Vector4, 0x0303> xwxw;
		Swizzle4<Vector4, 0x1303> ywxw;
		Swizzle4<Vector4, 0x2303> zwxw;
		Swizzle4<Vector4, 0x3303> wwxw;
		Swizzle4<Vector4, 0x0013> xxyw;
		Swizzle4<Vector4, 0x1013> yxyw;
		Swizzle4<Vector4, 0x2013> zxyw;
		Swizzle4<Vector4, 0x3013> wxyw;
		Swizzle4<Vector4, 0x0113> xyyw;
		Swizzle4<Vector4, 0x1113> yyyw;
		Swizzle4<Vector4, 0x2113> zyyw;
		Swizzle4<Vector4, 0x3113> wyyw;
		Swizzle4<Vector4, 0x0213> xzyw;
		Swizzle4<Vector4, 0x1213> yzyw;
		Swizzle4<Vector4, 0x2213> zzyw;
		Swizzle4<Vector4, 0x3213> wzyw;
		Swizzle4<Vector4, 0x0313> xwyw;
		Swizzle4<Vector4, 0x1313> ywyw;
		Swizzle4<Vector4, 0x2313> zwyw;
		Swizzle4<Vector4, 0x3313> wwyw;
		Swizzle4<Vector4, 0x0023> xxzw;
		Swizzle4<Vector4, 0x1023> yxzw;
		Swizzle4<Vector4, 0x2023> zxzw;
		Swizzle4<Vector4, 0x3023> wxzw;
		SwizzleMask4<Vector4, 0x0123> xyzw;
		Swizzle4<Vector4, 0x1123> yyzw;
		Swizzle4<Vector4, 0x2123> zyzw;
		Swizzle4<Vector4, 0x3123> wyzw;
		Swizzle4<Vector4, 0x0223> xzzw;
		Swizzle4<Vector4, 0x1223> yzzw;
		Swizzle4<Vector4, 0x2223> zzzw;
		Swizzle4<Vector4, 0x3223> wzzw;
		Swizzle4<Vector4, 0x0323> xwzw;
		Swizzle4<Vector4, 0x1323> ywzw;
		Swizzle4<Vector4, 0x2323> zwzw;
		Swizzle4<Vector4, 0x3323> wwzw;
		Swizzle4<Vector4, 0x0033> xxww;
		Swizzle4<Vector4, 0x1033> yxww;
		Swizzle4<Vector4, 0x2033> zxww;
		Swizzle4<Vector4, 0x3033> wxww;
		Swizzle4<Vector4, 0x0133> xyww;
		Swizzle4<Vector4, 0x1133> yyww;
		Swizzle4<Vector4, 0x2133> zyww;
		Swizzle4<Vector4, 0x3133> wyww;
		Swizzle4<Vector4, 0x0233> xzww;
		Swizzle4<Vector4, 0x1233> yzww;
		Swizzle4<Vector4, 0x2233> zzww;
		Swizzle4<Vector4, 0x3233> wzww;
		Swizzle4<Vector4, 0x0333> xwww;
		Swizzle4<Vector4, 0x1333> ywww;
		Swizzle4<Vector4, 0x2333> zwww;
		Swizzle4<Vector4, 0x3333> wwww;
	};
};

}  // namespace rr
