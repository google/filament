# RUN: llvm-mc -triple i386-pc-linux-gnu -filetype=obj -o %t %s
# RUN: llvm-readobj -s < %t | FileCheck %s
.section .nobits
.byte 1
.section .nobits2
.byte 1
.section .nobitsfoo
.byte 1
.section .init_array
.byte 1
.section .init_array2
.byte 1
.section .init_arrayfoo
.byte 1
.section .fini_array
.byte 1
.section .fini_array2
.byte 1
.section .fini_arrayfoo
.byte 1
.section .preinit_array
.byte 1
.section .preinit_array2
.byte 1
.section .preinit_arrayfoo
.byte 1
.section .note
.byte 1
.section .note2
.byte 1
.section .notefoo
.byte 1
# CHECK:        Name: .nobits
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .nobits2
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .nobitsfoo
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .init_array
# CHECK-NEXT:   Type:  SHT_INIT_ARRAY
# CHECK:        Name: .init_array2
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .init_arrayfoo
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .fini_array
# CHECK-NEXT:   Type: SHT_FINI_ARRAY
# CHECK:        Name: .fini_array2
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .fini_arrayfoo
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .preinit_array
# CHECK-NEXT:   Type: SHT_PREINIT_ARRAY
# CHECK:        Name: .preinit_array2
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .preinit_arrayfoo
# CHECK-NEXT:   Type: SHT_PROGBITS
# CHECK:        Name: .note
# CHECK-NEXT:   Type: SHT_NOTE
# CHECK:        Name: .note2
# CHECK-NEXT:   Type: SHT_NOTE
# CHECK:        Name: .notefoo
# CHECK-NEXT:   Type: SHT_NOTE
