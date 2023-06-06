#version 450

layout(location = 0) out float FragColor;

struct Test
{
	bool a;
	bvec2 b;
	bvec3 c;
	bvec4 d;
};

struct Test2
{
	bool a[2];
	bvec2 b[2];
	bvec3 c[2];
	bvec4 d[2];
	Test e;
};

void main()
{
	Test t;
	Test2 t2;

	t.a = true;
	t.b = bvec2(true, false);
	t.c = bvec3(true, false, true);
	t.d = bvec4(true, false, true, false);

	t2.a = bool[](true, false);
	t2.b = bvec2[](bvec2(true, false), bvec2(false, true));
	t2.c = bvec3[](bvec3(true, false, true), bvec3(false, true, false));
	t2.d = bvec4[](bvec4(true, false, true, false), bvec4(false, true, false, true));

	bool a = t.a;
	bvec2 b = t.b;
	bvec3 c = t.c;
	bvec4 d = t.d;

	bool a2[2] = t2.a;
	bvec2 b2[2] = t2.b;
	bvec3 c2[2] = t2.c;
	bvec4 d2[2] = t2.d;

	t = Test(true, bvec2(true, false), bvec3(true), bvec4(false));
	t2 = Test2(
			bool[](true, true),
			bvec2[](bvec2(true), bvec2(false)),
			bvec3[](bvec3(true), bvec3(false)),
			bvec4[](bvec4(true), bvec4(false)),
			Test(true, bvec2(true, false), bvec3(true), bvec4(false)));

	Test t3[2] = Test[](
			Test(true, bvec2(true, false), bvec3(true), bvec4(false)),
			Test(false, bvec2(false, true), bvec3(false), bvec4(true)));
}
