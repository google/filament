//===- subzero/src/IceMemory.cpp - Memory management definitions -*- C++ -*-==//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements memory management related routines for subzero.
/////
//===----------------------------------------------------------------------===//

#include "IceMemory.h"

#include "IceCfg.h"
#include "IceLiveness.h"
#include "IceTLS.h"

#include <cassert>
#include <utility>

namespace Ice {
ICE_TLS_DEFINE_FIELD(ArenaAllocator *, CfgAllocatorTraits, CfgAllocator);

CfgAllocatorTraits::allocator_type CfgAllocatorTraits::current() {
  return ICE_TLS_GET_FIELD(CfgAllocator);
}

void CfgAllocatorTraits::set_current(const manager_type *Manager) {
  ArenaAllocator *Allocator =
      Manager == nullptr ? nullptr : Manager->Allocator.get();
  set_current(Allocator);
}

void CfgAllocatorTraits::set_current(ArenaAllocator *Allocator) {
  ICE_TLS_SET_FIELD(CfgAllocator, Allocator);
}

void CfgAllocatorTraits::set_current(std::nullptr_t) {
  ICE_TLS_SET_FIELD(CfgAllocator, nullptr);
}

ICE_TLS_DEFINE_FIELD(ArenaAllocator *, LivenessAllocatorTraits,
                     LivenessAllocator);

LivenessAllocatorTraits::allocator_type LivenessAllocatorTraits::current() {
  return ICE_TLS_GET_FIELD(LivenessAllocator);
}

void LivenessAllocatorTraits::set_current(const manager_type *Manager) {
  ArenaAllocator *Allocator =
      Manager == nullptr ? nullptr : Manager->getAllocator();
  ICE_TLS_SET_FIELD(LivenessAllocator, Allocator);
}
} // end of namespace Ice
