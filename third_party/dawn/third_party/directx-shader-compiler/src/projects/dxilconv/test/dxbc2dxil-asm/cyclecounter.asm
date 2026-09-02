/*// RUN: %testasm %s /Fo %t.dxbc*/
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

ps_5_0
dcl_temps 1
dcl_output o0.xyzw
dcl_input vCycleCounter.x
mov r0, l(0,0,0,0)
mov r0.z, vCycleCounter.x
mov o0, r0
