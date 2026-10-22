///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ScopeNest.h                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
// This file holds the type used to represent a scope nest. The
// ScopeNestEvent is the type used by the iterator to represent the nesting
// structure in the cfg.
//
// The iterator returns tokens of type ScopeNestEvent that describe the
// structure. A ScopeNestEvent is a pair of a basic block and a scope type.
// The block may be null depending on the scope type so it should always be
// checked for null before using.
//
// See @ScopeNestIterator.h for more details on the iteration.
//
// The element types represent the major "events" that occur when walking a
// scope nest. The block field corresponds to the basic block where the
// event occurs. There may not always be a block associated with the event
// because some events are used just to indicate transitions. For example,
// with the If_Else and Switch_Case events, the actual else and case blocks
// will be returned with the next event, which will have its own type indicating
// the event caused by that block.
//
// The event location in the block depends on the scope type. For scope-opening
// events, the location is at the end of the block. For example, the @If_Begin
// event occurs an the end of the A block. For scope closing events the event
// occurs at the top of the block. For example, the @If_End event occurs at
// the entry to the X block. For events that do not open or close scopes
// the events generally occur at the bottom of the block. For example, the
// @Loop_Continue event occurs with the branch at the end of the block.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "llvm/Pass.h"

namespace llvm {

struct ScopeNestEvent {
  enum class Type {
    Invalid,        // Not a valid event.
    TopLevel_Begin, // Before the first block. Block will be null.
    Body,           // In the body of a scope. No interesting event.
    Switch_Begin,   // Begin a switch scope. Block has multi-way branch.
    Switch_Break,   // Break out of a switch scope. Block may be null.
    Switch_Case,    // A case will start at the next event. Block will be null.
    Switch_End,     // End a switch scope. Block is after all switch exits.
    Loop_Begin,     // Begin a loop scope. Block has one branch leading to loop
                    // header.
    Loop_Continue,  // A "continue" inside a loop. Block has one branch leading
                    // to loop latch.
    Loop_Break,     // A "break" inside a loop. Block has one branch leading to
                    // Loop_End block.
    Loop_End, // End of loop marker. Block is after the loop (the post loop
              // footer).
    If_Begin, // Start of if. Block has branch leading to the two sides of the
              // if.
    If_Else,  // The else body starts at the next event. Block will be null.
    If_End,   // The end if marker. Block may be null.
    TopLevel_End, // After the last block. Block will be null.
  };

  typedef const BasicBlock BlockTy; // TODO: make this a template so we can have
                                    // const and non-const iterators.
  Type ElementType;
  BlockTy *Block;

  ScopeNestEvent(BlockTy *B, Type T) : ElementType(T), Block(B) {}
  static ScopeNestEvent Invalid() {
    return ScopeNestEvent(nullptr, Type::Invalid);
  }

  bool IsBeginScope() const {
    switch (ElementType) {
    case Type::TopLevel_Begin:
      return "TopLevel_Begin";
    case Type::Switch_Begin:
      return "Switch_Begin";
    case Type::Loop_Begin:
      return "Loop_Begin";
    case Type::If_Begin:
      return "If_Begin";
      return true;
    }
    return false;
  }

  bool IsEndScope() const {
    switch (ElementType) {
    case Type::If_End:
    case Type::Switch_End:
    case Type::Loop_End:
    case Type::TopLevel_End:
      return true;
    }
    return false;
  }

  const char *GetElementTypeName() const {
    switch (ElementType) {
    case Type::Invalid:
      return "Invalid";
    case Type::TopLevel_Begin:
      return "TopLevel_Begin";
    case Type::Body:
      return "Body";
    case Type::Switch_Begin:
      return "Switch_Begin";
    case Type::Switch_Case:
      return "Switch_Case";
    case Type::Switch_Break:
      return "Switch_Break";
    case Type::Switch_End:
      return "Switch_End";
    case Type::Loop_Begin:
      return "Loop_Begin";
    case Type::Loop_Continue:
      return "Loop_Continue";
    case Type::Loop_Break:
      return "Loop_Break";
    case Type::Loop_End:
      return "Loop_End";
    case Type::If_Begin:
      return "If_Begin";
    case Type::If_Else:
      return "If_Else";
    case Type::If_End:
      return "If_End";
    case Type::TopLevel_End:
      return "TopLevel_End";
    }
    assert(false && "unreachable");
    return "Unknown";
  }

  bool operator==(const ScopeNestEvent &other) const {
    return Block == other.Block && ElementType == other.ElementType;
  }
};
} // namespace llvm
