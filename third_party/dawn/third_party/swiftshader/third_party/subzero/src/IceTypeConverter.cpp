//===- subzero/src/IceTypeConverter.cpp - Convert ICE/LLVM Types ----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements how to convert LLVM types to ICE types, and ICE types to
/// LLVM types.
///
//===----------------------------------------------------------------------===//

#include "IceTypeConverter.h"

#include "llvm/Support/raw_ostream.h"

namespace Ice {

TypeConverter::TypeConverter(llvm::LLVMContext &Context) {
  addLLVMType(IceType_void, llvm::Type::getVoidTy(Context));
  llvm::Type *Type_i1 = llvm::IntegerType::get(Context, 1);
  llvm::Type *Type_i8 = llvm::IntegerType::get(Context, 8);
  llvm::Type *Type_i16 = llvm::IntegerType::get(Context, 16);
  llvm::Type *Type_i32 = llvm::IntegerType::get(Context, 32);
  llvm::Type *Type_f32 = llvm::Type::getFloatTy(Context);
  addLLVMType(IceType_i1, Type_i1);
  addLLVMType(IceType_i8, Type_i8);
  addLLVMType(IceType_i16, Type_i16);
  addLLVMType(IceType_i32, Type_i32);
  addLLVMType(IceType_i64, llvm::IntegerType::get(Context, 64));
  addLLVMType(IceType_f32, Type_f32);
  addLLVMType(IceType_f64, llvm::Type::getDoubleTy(Context));
  addLLVMType(IceType_v4i1, llvm::VectorType::get(Type_i1, 4));
  addLLVMType(IceType_v8i1, llvm::VectorType::get(Type_i1, 8));
  addLLVMType(IceType_v16i1, llvm::VectorType::get(Type_i1, 16));
  addLLVMType(IceType_v16i8, llvm::VectorType::get(Type_i8, 16));
  addLLVMType(IceType_v8i16, llvm::VectorType::get(Type_i16, 8));
  addLLVMType(IceType_v4i32, llvm::VectorType::get(Type_i32, 4));
  addLLVMType(IceType_v4f32, llvm::VectorType::get(Type_f32, 4));
  assert(LLVM2IceMap.size() == static_cast<size_t>(IceType_NUM));
}

void TypeConverter::addLLVMType(Type Ty, llvm::Type *LLVMTy) {
  LLVM2IceMap[LLVMTy] = Ty;
}

Type TypeConverter::convertToIceTypeOther(llvm::Type *LLVMTy) const {
  switch (LLVMTy->getTypeID()) {
  case llvm::Type::PointerTyID:
  case llvm::Type::FunctionTyID:
    return getPointerType();
  default:
    return Ice::IceType_NUM;
  }
}

} // namespace Ice
