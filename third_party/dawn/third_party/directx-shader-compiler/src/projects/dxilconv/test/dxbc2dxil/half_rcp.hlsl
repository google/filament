// FXC command line: fxc /T ps_5_0 /Od %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

float main(float4 pos : SV_POSITION) : SV_Target
{
	min16float test = 0.5;
	min16float r = rcp(test);
	return r;
}
