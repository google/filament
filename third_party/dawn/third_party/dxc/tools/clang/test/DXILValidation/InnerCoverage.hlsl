// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// note: define GENLL in order to generate the basis for InnerCoverage.ll

// CHECK: error: Parameter with semantic SV_InnerCoverage has overlapping semantic index at 0
// CHECK: error: Pixel shader inputs SV_Coverage and SV_InnerCoverage are mutually exclusive

void main(snorm float b : B, uint c:C, 
#ifndef GENLL
	in uint inner : SV_InnerCoverage, in uint inner2 : SV_InnerCoverage,
#endif
	inout uint cover: SV_Coverage)
{
#ifndef GENLL
  cover = cover & c;
#else
  cover = cover & inner;
#endif
}
