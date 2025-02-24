##===- subzero/runtime/szrt_asm_x8632.s - Subzero runtime asm helpers------===##
##
##                        The Subzero Code Generator
##
## This file is distributed under the University of Illinois Open Source
## License. See LICENSE.TXT for details.
##
##===----------------------------------------------------------------------===##
##
## This file provides an assembly implementation of various helpers needed by
## the Subzero x8632 runtime.
##
##===----------------------------------------------------------------------===##

	.text
	.p2align 5,0xf4

	.ifdef NONSFI
	.globl __Sz_getIP_eax
__Sz_getIP_eax:
	movl (%esp), %eax
	ret

	.globl __Sz_getIP_ecx
__Sz_getIP_ecx:
	movl (%esp), %ecx
	ret

	.globl __Sz_getIP_edx
__Sz_getIP_edx:
	movl (%esp), %edx
	ret

	.globl __Sz_getIP_ebx
__Sz_getIP_ebx:
	movl (%esp), %ebx
	ret

	.globl __Sz_getIP_ebp
__Sz_getIP_ebp:
	movl (%esp), %ebp
	ret

	.globl __Sz_getIP_esi
__Sz_getIP_esi:
	movl (%esp), %esi
	ret

	.globl __Sz_getIP_edi
__Sz_getIP_edi:
	movl (%esp), %edi
	ret
	.endif  # NONSFI
