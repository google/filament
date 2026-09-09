// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

BlendState NoBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = FALSE;
};

BlendState A;
BlendState B = A;

DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};

float4 main(int4 a : A) : SV_TARGET
{
  RenderTargetView rtv { state=foo; };
  RenderTargetView  rt = rtv;
  return a + abs(a.yxxx);
}