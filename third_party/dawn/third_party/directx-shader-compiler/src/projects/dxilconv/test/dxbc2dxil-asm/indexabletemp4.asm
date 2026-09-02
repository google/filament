/*// RUN: %testasm %s /Fo %t.dxbc*/
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[12], dynamicIndexed
dcl_input_ps constant v1.x
dcl_input_ps constant v1.y
dcl_output o0.x
dcl_temps 1
dcl_indexableTemp x0[4], 2
mov r0.x, v1.x
mov x0[0].x, cb0[r0.x + 0].x
mov x0[1].x, cb0[r0.x + 4].x
mov r0.x, v1.y
mov x0[1].y, r0.x
mov o0.x, x0[ x0[1].y + 77 ].x
ret
