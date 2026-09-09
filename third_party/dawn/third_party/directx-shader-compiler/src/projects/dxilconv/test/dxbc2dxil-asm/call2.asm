/*// RUN: %testasm %s /Fo %t.dxbc*/
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

ps_5_0
dcl_globalFlags refactoringAllowed
dcl_input_ps linear v0.x
dcl_input_ps constant v1.xyz
dcl_output o0.x
dcl_temps 1
mov r0.xyz, v1.xyz
call l0
callc_nz r0.x, l0
switch v1.x
  case 1
  call l2
  callc_nz r0.y, l1
  break
  default
  callc_nz r0.z, l2
  break
  case 2
  break
endswitch
add o0.x, r0.x, l(1.000000)
ret
label l0
mov r0.x, l(5.000000)
ret
label l1
mov r0.x, v0.x
ret
label l2
mov r0.x, l(3.000000)
ret
