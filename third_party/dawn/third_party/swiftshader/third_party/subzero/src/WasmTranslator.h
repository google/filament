//===- subzero/src/WasmTranslator.h - WASM to Subzero Translation ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares a driver for translating Wasm bitcode into PNaCl bitcode.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_WASMTRANSLATOR_H
#define SUBZERO_SRC_WASMTRANSLATOR_H

#if ALLOW_WASM

#include "IceGlobalContext.h"
#include "IceTranslator.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/Support/StreamingMemoryObject.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

namespace v8 {
namespace internal {
class Zone;
namespace wasm {
struct FunctionBody;
} // end of namespace wasm
} // end of namespace internal
} // end of namespace v8

namespace Ice {

class WasmTranslator : public Translator {
  WasmTranslator() = delete;
  WasmTranslator(const WasmTranslator &) = delete;
  WasmTranslator &operator=(const WasmTranslator &) = delete;

  template <typename F = std::function<void(Ostream &)>> void log(F Fn) {
    if (BuildDefs::dump() && (getFlags().getVerbose() & IceV_Wasm)) {
      Fn(Ctx->getStrDump());
      Ctx->getStrDump().flush();
    }
  }

public:
  explicit WasmTranslator(GlobalContext *Ctx);

  void translate(const std::string &IRFilename,
                 std::unique_ptr<llvm::DataStreamer> InputStream);

  /// Translates a single Wasm function.
  ///
  /// Parameters:
  ///   Zone - an arena for the V8 code to allocate from.
  ///   Body - information about the function to translate
  std::unique_ptr<Cfg>
  translateFunction(v8::internal::Zone *Zone,
                    v8::internal::wasm::FunctionBody &Body);

private:
  std::vector<uint8_t> Buffer;
};
} // namespace Ice

#endif // ALLOW_WASM

#endif // SUBZERO_SRC_WASMTRANSLATOR_H
