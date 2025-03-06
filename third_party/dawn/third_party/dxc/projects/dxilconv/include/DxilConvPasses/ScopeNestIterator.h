///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ScopeNestIterator.h                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implementation of ScopeNestIterator class.                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// The ScopeNestIterator class iterates over a cfg that has been annoated with
// scope markers by the scopenestedcfg pass.
//
// The iterator produces a sequence of ScopeNestEvent tokens as it iterates
// over the cfg. The tokens describe the nesting structure of the cfg and
// the blocks that correspond to the nesting events. Each block will only be
// returned once by the iterator.
//
// Because each block is only returned once some events do not have an
// associated block (i.e. it will be nullptr). This is necessary to handle
// cases where a block has two logical events assocaited with it. For example,
// when a block is the start of an else branch but also starts a new nested if
// scope.
//
// For example, for a nested if-else like this:
//              A
//             /  \
//            B    C
//            |   /  \
//            |  D    E
//            |   \  /
//            |     F
//            \     /
//             \   /
//               X
// We would get an event sequence like this:
//
// @TopLevel_Begin (null)
// @If_Begin       (A)
//   @Body         (B)
// @If_Else        (null)
//   @IF_Begin     (C)
//     @Body       (D)
//   @If_Else      (null)
//     @Body       (E)
//   @If_End       (F)
// @If_End         (X)
// @TopLevel_End   (null)
//
// See @ScopeNest.h for details on the scope events.
//
// Note:
// This iterator is implemented in a header file with the intention that
// it will be made into a templated version to support both const and non-const
// iterators.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "DxilConvPasses/ScopeNest.h"
#include "DxilConvPasses/ScopeNestedCFG.h"
#include "dxc/Support/Global.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"

#include <stack>

namespace llvm {
class Function;
class BasicBlock;

// The ScopeNestIterator class is a heavy-weight iterator that walks the cfg
// in scope nest order. The iterator keeps a large amount of state while
// iterating and so it is expensive to copy and compare iterators for equality.
// The end() iterator has no state so comparing and copying the end is
// relatively cheap.
//
// The iterator provides a standard c++ iterator interface. All the logic is
// handled by the IteratorState class.
//

class ScopeNestIterator {
public:
  typedef ScopeNestEvent::BlockTy Block; // TODO: make this a template.

  static ScopeNestIterator begin(const Function &F) {
    return ScopeNestIterator(F.getEntryBlock());
  }

  static ScopeNestIterator end() { return ScopeNestIterator(); }

  ScopeNestEvent &operator*() {
    DXASSERT_NOMSG(!m_state.IsDone());
    return m_currentElement;
  }

  ScopeNestIterator &operator++() {
    (void)GetNextElement();
    return *this;
  }

  bool operator==(const ScopeNestIterator &other) const {
    return m_state == other.m_state;
  }

  bool operator!=(const ScopeNestIterator &other) const {
    return !(*this == other);
  }

private: // Interface
  ScopeNestIterator(Block &entry)
      : m_state(&entry), m_currentElement(ScopeNestEvent::Invalid()) {
    // Must advance iterator to first element. Should always succeed.
    bool ok = GetNextElement();
    DXASSERT_LOCALVAR_NOMSG(ok, ok);
  }

  ScopeNestIterator()
      : m_state(nullptr), m_currentElement(ScopeNestEvent::Invalid()) {}

  bool GetNextElement() {
    bool ok = m_state.MoveNext();
    if (ok) {
      m_currentElement = m_state.GetCurrent();
    }
    return ok;
  }

private: // ScopeNestIterator Implementation
  // BranchAnnotation
  //
  // Provides safe access to the scope annotation on the block. Use
  // the operator bool() to check if there is an annotation.
  // e.g. if (BranchAnnotation a = BranchAnnotation::Read(B)) { ... }
  class BranchAnnotation {
  public:
    static BranchAnnotation Read(Block *B) {
      if (!B) {
        return BranchAnnotation();
      }

      const TerminatorInst *end = B->getTerminator();
      if (!end) {
        DXASSERT_NOMSG(false);
        return BranchAnnotation();
      }

      MDNode *md = end->getMetadata("dx.BranchKind");
      if (!md) {
        return BranchAnnotation();
      }

      BranchKind kind = static_cast<BranchKind>(
          cast<ConstantInt>(
              cast<ConstantAsMetadata>(md->getOperand(0))->getValue())
              ->getZExtValue());
      return BranchAnnotation(kind);
    }

    BranchAnnotation(BranchKind kind) : Kind(kind) {}

    operator bool() const { return IsSome(); }

    BranchKind Get() const {
      DXASSERT_NOMSG(IsSome());
      return Kind;
    }

    bool IsEndIf() const { return Kind == BranchKind::IfEnd; }

    bool IsEndScope() {
      switch (Kind) {
      case BranchKind::IfEnd:
      case BranchKind::LoopBreak:
      case BranchKind::LoopContinue:
      case BranchKind::LoopBackEdge:
      case BranchKind::LoopExit:
      case BranchKind::SwitchBreak:
      case BranchKind::SwitchEnd:
        return true;
      default:
        return false;
      }
    }

    bool IsBeginScope() {
      switch (Kind) {
      case BranchKind::IfBegin:
      case BranchKind::IfNoEnd:
      case BranchKind::LoopBegin:
      case BranchKind::LoopNoEnd:
      case BranchKind::SwitchBegin:
      case BranchKind::SwitchNoEnd:
        return true;
      default:
        return false;
      }
    }

    // Translate a branch annoatation to the corresponding event type.
    ScopeNestEvent::Type TranslateToNestType() {
      switch (Kind) {
      case BranchKind::Invalid:
        return ScopeNestEvent::Type::Invalid;

      case BranchKind::IfBegin:
        return ScopeNestEvent::Type::If_Begin;
      case BranchKind::IfEnd:
        return ScopeNestEvent::Type::If_End;
      case BranchKind::IfNoEnd:
        return ScopeNestEvent::Type::If_Begin;

      case BranchKind::SwitchBegin:
        return ScopeNestEvent::Type::Switch_Begin;
      case BranchKind::SwitchEnd:
        return ScopeNestEvent::Type::Switch_End;
      case BranchKind::SwitchNoEnd:
        return ScopeNestEvent::Type::Switch_Begin;
      case BranchKind::SwitchBreak:
        return ScopeNestEvent::Type::Switch_Break;

      case BranchKind::LoopBegin:
        return ScopeNestEvent::Type::Loop_Begin;
      case BranchKind::LoopExit:
        return ScopeNestEvent::Type::Loop_End;
      case BranchKind::LoopNoEnd:
        return ScopeNestEvent::Type::Loop_Begin;
      case BranchKind::LoopBreak:
        return ScopeNestEvent::Type::Loop_Break;
      case BranchKind::LoopContinue:
        return ScopeNestEvent::Type::Loop_Continue;
      case BranchKind::LoopBackEdge:
        return ScopeNestEvent::Type::Body; // End of loop is marked at loop
                                           // exit.
      }
      DXASSERT(false, "unreachable");
      return ScopeNestEvent::Type::Invalid;
    }

  private:
    bool IsSome() const { return Kind != BranchKind::Invalid; }
    BranchAnnotation() : Kind(BranchKind::Invalid) {}
    BranchKind Kind;
  };

  // Scope
  //
  // A nested scope. Used as part of the stack state to keep track of what
  // kind of scopes we have entered but not yet exited.
  //
  // Instead of using a class heirarchy we provide scope-specific methods
  // and validate the scope type to ensure that we only operate on the kind
  // of scope we expect to see.
  struct Scope {
    enum class Type { TopLevel, If, Loop, Switch };

  public:
    Scope(Type scopeType, Block *startBlock, BranchKind annotation)
        : m_type(scopeType), m_startAnnotation(annotation),
          m_startBlock(startBlock), m_endBlock(nullptr), m_backedge(nullptr) {
      if (m_type == Type::If) {
        DXASSERT_NOMSG(startBlock &&
                       startBlock->getTerminator()->getNumSuccessors() == 2);
      }
    }

    Type GetType() const { return m_type; }

    Block *GetStartBlock() { return m_startBlock; }
    Block *GetIfEndBlock() { return GetEndBlock(Type::If); }
    Block *GetLoopBackedgeBlock() {
      AssertType(Type::Loop);
      return m_backedge;
    }
    Block *GetLoopEndBlock() { return GetEndBlock(Type::Loop); }
    Block *GetSwitchEndBlock() { return GetEndBlock(Type::Switch); }

    void SetIfEndBlock(Block *B) { SetEndBlock(Type::If, B); }

    void SetLoopBackedgeBlock(Block *B) { SetBackedgeBlock(Type::Loop, B); }

    void SetLoopEndBlock(Block *B) { SetEndBlock(Type::Loop, B); }

    void SetSwitchEndBlock(Block *B) { SetEndBlock(Type::Switch, B); }

    bool operator==(const Scope &other) const {
      return m_type == other.m_type &&
             m_startAnnotation == other.m_startAnnotation &&
             m_startBlock == other.m_startBlock &&
             m_endBlock == other.m_endBlock && m_backedge == other.m_backedge;
    }

  private:
    Type m_type;
    BranchKind m_startAnnotation;
    Block *m_startBlock;
    Block *m_endBlock;
    Block *m_backedge; // only for loop.

    void SetEndBlock(Type expectedType, Block *endBlock) {
      AssertType(expectedType);
      AssertUnchanged(m_endBlock, endBlock);
      m_endBlock = endBlock;
    }

    void SetBackedgeBlock(Type expectedType, Block *backedge) {
      AssertType(expectedType);
      AssertUnchanged(m_backedge, backedge);
      m_backedge = backedge;
    }

    void AssertUnchanged(Block *oldBlock, Block *newBlock) {
      DXASSERT((oldBlock == nullptr || oldBlock == newBlock),
               "block should not change");
    }

    void AssertType(Type t) { DXASSERT_NOMSG(t == m_type); }

    Block *GetEndBlock(Type t) {
      AssertType(t);
      return m_endBlock;
    }
  };

  // StackState
  //
  // Keeps track of the state of exploration for an open scope. Uses a small
  // state machine to move through the exploration stages. When moving to a
  // new state it notifies the caller of the new state and any block associated
  // with the state.
  //
  // Transitions:
  //
  // Top Level
  // -------------------------------
  // Start     -> Top_begin
  // Top_begin -> Top_body
  // Top_body  -> Top_end
  // Top_end   -> Done
  //
  // If
  // -------------------------------
  // If_thenbody -> If_else | If_end
  // If_else     -> If_elsebody
  // If_elsebody -> If_end
  //
  // Loop
  // -------------------------------
  // Loop_body     -> Loop_backedge
  // Loop_backedge -> Loop_end
  //
  // Switch
  // -------------------------------
  // Switch_begin -> Switch_case
  // Switch_case  -> Switch_body
  // Switch_body  -> Switch_break
  // Switch_break -> Switch_case | Switch_end
  //
  //
  // Terminal States:
  // Done, Switch_end, Loop_end, If_end
  //
  class StackState {
  public:
    enum State {
      // Initial top level state before emitting the Top_begin token.
      Start,

      // If
      If_thenbody, // Exploring the true branch of the if.
      If_else,     // Transitioning from true to false branch.
      If_elsebody, // Exploring the false branch of the if.
      If_end,      // Finished exploring the if.

      // Loop
      Loop_body,     // Exploring the loop body.
      Loop_backedge, // On the loop latch block (branch to loop header).
      Loop_end,      // Finshed exploring the loop.

      // Switch
      Switch_begin, // Start of switch before entering any case.
      Switch_case,  // Starting a new case.
      Switch_body,  // Exploring a case body.
      Switch_break, // Break from a case.
      Switch_end,   // Finished exploring the switch.

      // Top level
      Top_begin, // Before exploring the first block.
      Top_body,  // Exploring the body of the function.
      Top_end,   // After exploring all blocks.

      // Final state after top level is popped.
      Done
    };

    StackState(Scope scope, unsigned edge)
        : m_scope(scope), m_edgeNumber(edge) {
      switch (scope.GetType()) {
      case Scope::Type::If:
        m_state = If_thenbody;
        break;
      case Scope::Type::Loop:
        m_state = Loop_body;
        break;
      case Scope::Type::Switch:
        m_state = Switch_begin;
        break;
      case Scope::Type::TopLevel:
        m_state = Start;
        break;
      default:
        DXASSERT_NOMSG(false);
      }
    }

    Scope &GetScope() { return m_scope; }
    const Scope &GetScope() const { return m_scope; }

    struct StateTransition {
      State state;
      Block *block;
    };

    // Transition this stack element to the next state and return associated
    // block.
    StateTransition MoveToNextState() {
      Block *block = nullptr;
      switch (m_state) {
      // IF
      case If_thenbody: {
        // See if we have an else body or not.
        // The else body is missing when:
        //   Case 1: Successor block is the found endif block.
        //   Case 2: Endif block was not found and successor is marked as an
        //   endif block.
        Block *succ = GetNextSuccessor();
        BranchAnnotation annotation = BranchAnnotation::Read(succ);

        const bool succMatchesFoundEndIf = (succ == m_scope.GetIfEndBlock());
        const bool succIsMarkedAsEndIf =
            (m_scope.GetIfEndBlock() == nullptr && annotation &&
             annotation.Get() == BranchKind::IfEnd);
        const bool succIsEndif = succMatchesFoundEndIf || succIsMarkedAsEndIf;

        if (succIsEndif) {
          m_state = If_end;
          block = succ;
        } else {
          m_state = If_else;
          block = nullptr;
        }
        break;
      }
      case If_else:
        m_state = If_elsebody;
        block = MoveToNextSuccessor();
        break;
      case If_elsebody:
        m_state = If_end;
        block = m_scope.GetIfEndBlock();
        break;

      // LOOP
      case Loop_body:
        m_state = Loop_backedge;
        block = m_scope.GetLoopBackedgeBlock();
        break;
      case Loop_backedge:
        m_state = Loop_end;
        block = m_scope.GetLoopEndBlock();
        break;

      // SWITCH
      case Switch_begin:
        m_state = Switch_case;
        block = nullptr;
        break;
      case Switch_case:
        block = GetCurrentSuccessor();
        m_state = Switch_body;
        break;
      case Switch_body:
        m_state = Switch_break;
        block = nullptr;
        break;
      case Switch_break:
        block = MoveToNextUniqueSuccessor();
        if (block) {
          m_state = Switch_case;
          block = nullptr; // will resume after emitting case marker.
        } else {
          m_state = Switch_end;
          block = m_scope.GetSwitchEndBlock();
        }
        break;

      // TOP LEVEL
      case Start:
        m_state = Top_begin;
        block = nullptr;
        break;

      case Top_begin:
        m_state = Top_body;
        block = m_scope.GetStartBlock();
        break;

      case Top_body:
        m_state = Top_end;
        block = nullptr;
        break;

      case Top_end:
        m_state = Done;
        block = nullptr;
        break;

      // INVALID
      // The stack state should already be popped because there is no next
      // state.
      case If_end:
      case Switch_end:
      case Loop_end:
      case Done:
      default:
        DXASSERT_NOMSG(false);
      }

      return {m_state, block};
    }

    bool operator==(const StackState &other) const {
      return m_scope == other.m_scope && m_edgeNumber == other.m_edgeNumber &&
             m_state == other.m_state;
    }

  private:
    Scope m_scope;
    unsigned m_edgeNumber;
    State m_state;

  private:
    // Return next successor or nullptr if no more successors need to be
    // explored. Does not modify current edge number.
    Block *GetNextSuccessor() { return GetSuccessor(m_edgeNumber + 1); }

    // Increment edge number and return next successor.
    Block *MoveToNextSuccessor() {
      Block *succ = GetNextSuccessor();
      if (succ) {
        ++m_edgeNumber;
      }
      return succ;
    }

    // Get the successor we are currently set to explore.
    Block *GetCurrentSuccessor() { return GetSuccessor(m_edgeNumber); }

    // Get successor block or nullptr if there is no such succssor.
    Block *GetSuccessor(unsigned succNumber) {
      Block *const scopeStartBlock = m_scope.GetStartBlock();
      Block *succ = nullptr;
      if (scopeStartBlock && scopeStartBlock->getTerminator()) {
        if (succNumber < scopeStartBlock->getTerminator()->getNumSuccessors()) {
          succ_const_iterator succs = succ_begin(scopeStartBlock);
          std::advance(succs, succNumber);
          succ = *succs;
        }
      }
      return succ;
    }

    // Move to the next succssor that does not match a previous successor.
    // Needed to avoid visiting blocks multiple times blocks in a switch
    // when multiple cases point to the same block.
    Block *MoveToNextUniqueSuccessor() {
      Block *succ = nullptr;

      SmallPtrSet<Block *, 8> visited;
      Block *const scopeStartBlock = m_scope.GetStartBlock();

      if (scopeStartBlock && scopeStartBlock->getTerminator()) {
        succ_const_iterator succs = succ_begin(scopeStartBlock);
        succ_const_iterator succsEnd = succ_end(scopeStartBlock);
        const unsigned nextEdgeNumber = m_edgeNumber + 1;
        unsigned edge = 0;
        // Mark all successors less than the current edge number as visited.
        for (; succs != succsEnd && edge < nextEdgeNumber; ++succs, ++edge) {
          visited.insert(*succs);
        }
        DXASSERT_NOMSG(succs == succsEnd || edge == nextEdgeNumber);

        // Look for next unvisited edge.
        for (; succs != succsEnd; ++succs, ++edge) {
          if (!visited.count(*succs)) {
            break;
          }
        }

        // If we found an edge before the end then move to it.
        if (succs != succsEnd) {
          DXASSERT_NOMSG(edge <
                         scopeStartBlock->getTerminator()->getNumSuccessors());
          succ = *succs;
          m_edgeNumber = edge;
        }
      }

      return succ;
    }
  };

  // ScopeStack
  //
  // A stack to hold state information about scopes that are under exploration.
  //
  class ScopeStack {
  public:
    bool Empty() const { return m_stack.empty(); }

    void Clear() { m_stack.clear(); }

    void PushScope(const Scope &scope) {
      m_stack.push_back(StackState(scope, 0));
    }

    void PopScope() {
      DXASSERT_NOMSG(!Empty());
      m_stack.pop_back();
    }

    Scope &Top() {
      DXASSERT_NOMSG(!Empty());
      return m_stack.back().GetScope();
    }

    const Scope &Top() const {
      DXASSERT_NOMSG(!Empty());
      return m_stack.back().GetScope();
    }

    // Transition state on the top of the stack to the next state.
    StackState::StateTransition AdvanceTopOfStack() {
      DXASSERT_NOMSG(!Empty());
      return m_stack.back().MoveToNextState();
    }

    Scope &FindInnermostLoop() { return FindInnermost(Scope::Type::Loop); }

    Scope &FindInnermostIf() { return FindInnermost(Scope::Type::If); }

    Scope &FindInnermostSwitch() { return FindInnermost(Scope::Type::Switch); }

    // Define equality to be fast for comparing to the "end" state
    // so that the iterator test in a loop is fase.
    bool operator==(const ScopeStack &other) const {
      // Quick check on size to make non-equality fast.
      return m_stack.size() == other.m_stack.size() && m_stack == other.m_stack;
    }

  private:
    typedef std::vector<StackState> Stack;
    Stack m_stack;

    Scope &FindInnermost(Scope::Type type) {
      Stack::reverse_iterator scope = std::find_if(
          m_stack.rbegin(), m_stack.rend(), [type](const StackState &s) {
            return s.GetScope().GetType() == type;
          });
      DXASSERT_NOMSG(scope != m_stack.rend());
      return scope->GetScope();
    }
  };

  // IteratorState
  //
  // Keeps track of all the current state of the iteration. The iterator state
  // works as follows.
  //
  // We keep a current event that describes the most recent event returned by
  // the iterator. To advance the iterator we look at whether we have a valid
  // block associated with the event. If we do then we keep exploring from
  // that block. If there is no block (i.e. it is nullptr) then we explore from
  // the top element of the scope stack.
  //
  // The scope stack is used to keep track of the nested scopes. The stack
  // elements are a little state machine that keep track of what the next action
  // should be when exploring from the stack. The last action is the "end scope"
  // action which tells us we should pop the scope from the stack.
  class IteratorState {
  public:
    IteratorState(Block *entry)
        : m_current(ScopeNestEvent::Invalid()), m_stack() {
      if (entry) {
        m_stack.PushScope(
            Scope(Scope::Type::TopLevel, entry, BranchKind::Invalid));
      } else {
        SetDone();
      }
    }

    ScopeNestEvent GetCurrent() { return m_current; }

    // Move to the next event.
    // Return true if there is a new valid event or false if there is no more
    // events.
    bool MoveNext() {
      if (IsDone()) {
        return false;
      }

      if (m_current.Block == nullptr) {
        MoveFromTopOfStack();
      } else {
        MoveFromCurrentBlock();
      }
      return !IsDone();
    }

    bool IsDone() { return m_stack.Empty() && m_current.Block == nullptr; }

    bool operator==(const IteratorState &other) const {
      return m_current == other.m_current && m_stack == other.m_stack;
    }

  private:
    ScopeNestEvent m_current;
    ScopeStack m_stack;

  private:
    void SetDone() {
      m_stack.Clear();
      m_current = ScopeNestEvent::Invalid();
      DXASSERT_NOMSG(IsDone());
    }

    void SetCurrent(ScopeNestEvent::Type T, Block *B) {
      m_current.ElementType = T;
      m_current.Block = B;

      if (B) {
        BranchAnnotation annotation = BranchAnnotation::Read(B);
        if (annotation) {
          DXASSERT_NOMSG(annotation.TranslateToNestType() == T);
        }
      }
    }

    void MoveFromTopOfStack() {
      DXASSERT_NOMSG(!m_stack.Empty());
      StackState::StateTransition next = m_stack.AdvanceTopOfStack();
      switch (next.state) {
      case StackState::If_else:
        DXASSERT_NOMSG(next.block == nullptr);
        SetCurrent(ScopeNestEvent::Type::If_Else, next.block);
        break;
      case StackState::If_elsebody:
        EnterScopeBodyFromStack(next.block);
        break;
      case StackState::If_end:
        m_stack.PopScope();
        SetCurrent(ScopeNestEvent::Type::If_End, next.block);
        break;

      case StackState::Loop_backedge:
        SetCurrent(
            BranchAnnotation(BranchKind::LoopBackEdge).TranslateToNestType(),
            next.block);
        break;
      case StackState::Loop_end:
        m_stack.PopScope();
        SetCurrent(ScopeNestEvent::Type::Loop_End, next.block);
        break;

      case StackState::Switch_case:
        DXASSERT_NOMSG(next.block == nullptr);
        SetCurrent(ScopeNestEvent::Type::Switch_Case, next.block);
        break;

      case StackState::Switch_body:
        EnterScopeBodyFromStack(next.block);
        break;

      case StackState::Switch_break:
        DXASSERT_NOMSG(next.block == nullptr);
        SetCurrent(ScopeNestEvent::Type::Switch_Break, next.block);
        break;

      case StackState::Switch_end:
        m_stack.PopScope();
        SetCurrent(ScopeNestEvent::Type::Switch_End, next.block);
        break;

      case StackState::Top_begin:
        DXASSERT_NOMSG(next.block == nullptr);
        SetCurrent(ScopeNestEvent::Type::TopLevel_Begin, next.block);
        break;

      case StackState::Top_body:
        EnterScopeBodyFromStack(next.block);
        break;

      case StackState::Top_end:
        DXASSERT_NOMSG(next.block == nullptr);
        SetCurrent(ScopeNestEvent::Type::TopLevel_End, next.block);
        break;

      case StackState::Done:
        m_stack.PopScope();
        SetDone();
        break;

      default:
        DXASSERT_NOMSG(false);
      }
    }

    void EnterScopeBodyFromStack(Block *B) {
      DXASSERT_NOMSG(B);
      BranchAnnotation annotation = BranchAnnotation::Read(B);
      if (annotation) {
        // Make sure we are not ending a scope end because that will cause
        // us to move from the stack again. Indicates some problem with the
        // state transition.
        // Allow SwitchEnd for empty case
        BranchKind Kind = annotation.Get();
        DXASSERT_LOCALVAR_NOMSG(Kind, Kind != BranchKind::IfEnd &&
                                          Kind != BranchKind::LoopBackEdge &&
                                          Kind != BranchKind::LoopExit);
      }
      MoveToBlock(B);
    }

    void MoveFromCurrentBlock() {
      DXASSERT_NOMSG(m_current.Block && m_current.Block->getTerminator());
      BranchAnnotation annotation = BranchAnnotation::Read(m_current.Block);

      if (annotation) {
        MoveFromAnnotatedBlock(annotation.Get());
      } else {
        MoveFromNonAnnotatedBlock();
      }
    }

    void MoveFromAnnotatedBlock(BranchKind annotation) {
      switch (annotation) {
      // Already entered a new scope.
      case BranchKind::IfBegin:
      case BranchKind::IfNoEnd:
      case BranchKind::LoopBegin:
      case BranchKind::LoopNoEnd:
        DXASSERT(m_current.Block->getTerminator()->getNumSuccessors() >= 1,
                 "scope entry should have a successor");
        MoveToFirstSuccessor();
        break;

      // Start switch. Need to emit first case element from stack.
      case BranchKind::SwitchBegin:
      case BranchKind::SwitchNoEnd:
        MoveFromTopOfStack();
        break;

      // Already exited an old scope.
      case BranchKind::IfEnd:
      case BranchKind::SwitchEnd:
      case BranchKind::LoopExit:
        DXASSERT(m_current.Block->getTerminator()->getNumSuccessors() <= 1,
                 "scope exit should not have multiple successors");
        MoveToFirstSuccessor();
        break;

      // Keep exploring in same scope.
      case BranchKind::SwitchBreak:
      case BranchKind::LoopBreak:
      case BranchKind::LoopContinue:
      case BranchKind::LoopBackEdge:
        MoveFromTopOfStack();
        break;

      default:
        DXASSERT_NOMSG(false);
      }
    }

    void MoveFromNonAnnotatedBlock() {
      DXASSERT(m_current.Block->getTerminator()->getNumSuccessors() <= 1,
               "multi-way branch should be annotated");
      MoveToFirstSuccessor();
    }

    void MoveToFirstSuccessor() {
      // No successors to explore. Continue from current scope.
      if (!m_current.Block->getTerminator()->getNumSuccessors()) {
        DXASSERT_NOMSG(isa<ReturnInst>(m_current.Block->getTerminator()));
        MoveFromTopOfStack();
        return;
      }

      // Get first successor block.
      Block *succ = *succ_const_iterator(m_current.Block->getTerminator());
      MoveToBlock(succ);
    }

    void MoveToBlock(Block *B) {
      // Annotated successor.
      if (BranchAnnotation annotation = BranchAnnotation::Read(B)) {
        if (annotation.IsEndScope()) {
          EnterEndOfScope(B, annotation.Get());
        } else {
          DXASSERT_NOMSG(annotation.IsBeginScope());
          StartNewScope(B, annotation.Get());
        }
      }
      // Non-Annotated successor.
      else {
        SetCurrent(ScopeNestEvent::Type::Body, B);
      }
    }

    // Visit the end of scope node from a predecssor we have already explored.
    void EnterEndOfScope(Block *endOfScopeBlock, BranchKind endofScopeKind) {
      switch (endofScopeKind) {
      case BranchKind::IfEnd: {
        Scope &ifScope = m_stack.FindInnermostIf();
        ifScope.SetIfEndBlock(endOfScopeBlock);
        MoveFromTopOfStack();
        break;
      }

      case BranchKind::LoopBackEdge: {
        Scope &loopScope = m_stack.FindInnermostLoop();
        loopScope.SetLoopBackedgeBlock(endOfScopeBlock);
        MoveFromTopOfStack();
        break;
      }

      case BranchKind::LoopExit: {
        Scope &loopScope = m_stack.FindInnermostLoop();
        loopScope.SetLoopEndBlock(endOfScopeBlock);
        MoveFromTopOfStack();
        break;
      }

      case BranchKind::SwitchEnd: {
        Scope &switchScope = m_stack.FindInnermostSwitch();
        switchScope.SetSwitchEndBlock(endOfScopeBlock);
        MoveFromTopOfStack();
        break;
      }

      case BranchKind::LoopBreak: {
        Scope &loopScope = m_stack.FindInnermostLoop();
        loopScope.SetLoopEndBlock(endOfScopeBlock->getUniqueSuccessor());
        SetCurrent(ScopeNestEvent::Type::Loop_Break, endOfScopeBlock);
        break;
      }

      case BranchKind::LoopContinue: {
        Scope &loopScope = m_stack.FindInnermostLoop();
        loopScope.SetLoopBackedgeBlock(endOfScopeBlock->getUniqueSuccessor());
        SetCurrent(ScopeNestEvent::Type::Loop_Continue, endOfScopeBlock);
        break;
      }

      case BranchKind::SwitchBreak: {
        Scope &switchScope = m_stack.FindInnermostSwitch();
        switchScope.SetSwitchEndBlock(endOfScopeBlock->getUniqueSuccessor());
        SetCurrent(ScopeNestEvent::Type::Switch_Break, endOfScopeBlock);
        break;
      }

      default:
        DXASSERT_NOMSG(false);
      }
    }

    void StartNewScope(Block *startOfScopeBlock, BranchKind startOfScopeKind) {
      Scope::Type scopeType = Scope::Type::TopLevel;
      ScopeNestEvent::Type nestType = ScopeNestEvent::Type::Invalid;
      switch (startOfScopeKind) {
      case BranchKind::IfBegin:
      case BranchKind::IfNoEnd:
        scopeType = Scope::Type::If;
        nestType = ScopeNestEvent::Type::If_Begin;
        break;
      case BranchKind::LoopBegin:
      case BranchKind::LoopNoEnd:
        scopeType = Scope::Type::Loop;
        nestType = ScopeNestEvent::Type::Loop_Begin;
        break;
      case BranchKind::SwitchBegin:
      case BranchKind::SwitchNoEnd:
        scopeType = Scope::Type::Switch;
        nestType = ScopeNestEvent::Type::Switch_Begin;
        break;
      default:
        DXASSERT_NOMSG(false);
      }

      SetCurrent(nestType, startOfScopeBlock);
      m_stack.PushScope(Scope(scopeType, startOfScopeBlock, startOfScopeKind));
    }
  };

private: // Members
  IteratorState m_state;
  ScopeNestEvent m_currentElement;
};

} // namespace llvm
