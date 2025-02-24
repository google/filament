/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Intrinsic Function Source Fragment                                         *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_IR_INTRINSIC_WASM_ENUMS_H
#define LLVM_IR_INTRINSIC_WASM_ENUMS_H

namespace llvm {
namespace Intrinsic {
enum WASMIntrinsics : unsigned {
// Enum values for intrinsics
    wasm_alltrue = 6285,                              // llvm.wasm.alltrue
    wasm_anytrue,                              // llvm.wasm.anytrue
    wasm_atomic_notify,                        // llvm.wasm.atomic.notify
    wasm_atomic_wait_i32,                      // llvm.wasm.atomic.wait.i32
    wasm_atomic_wait_i64,                      // llvm.wasm.atomic.wait.i64
    wasm_avgr_unsigned,                        // llvm.wasm.avgr.unsigned
    wasm_bitselect,                            // llvm.wasm.bitselect
    wasm_data_drop,                            // llvm.wasm.data.drop
    wasm_dot,                                  // llvm.wasm.dot
    wasm_extract_exception,                    // llvm.wasm.extract.exception
    wasm_get_ehselector,                       // llvm.wasm.get.ehselector
    wasm_get_exception,                        // llvm.wasm.get.exception
    wasm_landingpad_index,                     // llvm.wasm.landingpad.index
    wasm_lsda,                                 // llvm.wasm.lsda
    wasm_memory_grow,                          // llvm.wasm.memory.grow
    wasm_memory_init,                          // llvm.wasm.memory.init
    wasm_memory_size,                          // llvm.wasm.memory.size
    wasm_narrow_signed,                        // llvm.wasm.narrow.signed
    wasm_narrow_unsigned,                      // llvm.wasm.narrow.unsigned
    wasm_qfma,                                 // llvm.wasm.qfma
    wasm_qfms,                                 // llvm.wasm.qfms
    wasm_rethrow_in_catch,                     // llvm.wasm.rethrow.in.catch
    wasm_sub_saturate_signed,                  // llvm.wasm.sub.saturate.signed
    wasm_sub_saturate_unsigned,                // llvm.wasm.sub.saturate.unsigned
    wasm_swizzle,                              // llvm.wasm.swizzle
    wasm_throw,                                // llvm.wasm.throw
    wasm_tls_align,                            // llvm.wasm.tls.align
    wasm_tls_base,                             // llvm.wasm.tls.base
    wasm_tls_size,                             // llvm.wasm.tls.size
    wasm_trunc_saturate_signed,                // llvm.wasm.trunc.saturate.signed
    wasm_trunc_saturate_unsigned,              // llvm.wasm.trunc.saturate.unsigned
    wasm_trunc_signed,                         // llvm.wasm.trunc.signed
    wasm_trunc_unsigned,                       // llvm.wasm.trunc.unsigned
    wasm_widen_high_signed,                    // llvm.wasm.widen.high.signed
    wasm_widen_high_unsigned,                  // llvm.wasm.widen.high.unsigned
    wasm_widen_low_signed,                     // llvm.wasm.widen.low.signed
    wasm_widen_low_unsigned,                   // llvm.wasm.widen.low.unsigned
}; // enum
} // namespace Intrinsic
} // namespace llvm

#endif
