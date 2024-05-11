#version 310 es
precision mediump float;
layout(location = 0) out float FragColor;
layout(location = 0) flat in int index;

const float LUT[16] = float[](
	1.0, 2.0, 3.0, 4.0,
	1.0, 2.0, 3.0, 4.0,
	1.0, 2.0, 3.0, 4.0,
	1.0, 2.0, 3.0, 4.0);

void main()
{
	// Try reading LUTs, both in branches and not branch.
	FragColor = LUT[index];
	if (index < 10)
		FragColor += LUT[index ^ 1];
	else
		FragColor += LUT[index & 1];

	// Not declared as a LUT, but can be promoted to one.
	vec4 foo[4] = vec4[](vec4(0.0), vec4(1.0), vec4(8.0), vec4(5.0));
	if (index > 30)
	{
		FragColor += foo[index & 3].y;
	}
	else
	{
		FragColor += foo[index & 1].x;
	}

	// Not declared as a LUT, but this cannot be promoted, because we have a partial write.
	vec4 foobar[4] = vec4[](vec4(0.0), vec4(1.0), vec4(8.0), vec4(5.0));
	if (index > 30)
	{
		foobar[1].z = 20.0;
	}
	FragColor += foobar[index & 3].z;

	// Not declared as a LUT, but this cannot be promoted, because we have two complete writes.
	vec4 baz[4] = vec4[](vec4(0.0), vec4(1.0), vec4(8.0), vec4(5.0));
	baz = vec4[](vec4(20.0), vec4(30.0), vec4(50.0), vec4(60.0));
	FragColor += baz[index & 3].z;
}
