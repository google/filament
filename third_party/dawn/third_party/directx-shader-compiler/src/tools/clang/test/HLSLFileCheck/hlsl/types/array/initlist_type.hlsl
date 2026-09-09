// RUN: %dxc -E main -T ps_6_0 %s

float4 main(int4 b:B) : SV_TARGET
{
	float m[2][2] =
	{
		{ -1., 3.2 },
		{ 6.2, 0.1 }
	};

  return m[b.x][b.y];
}