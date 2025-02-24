//===- subzero/src/IceInstVarIter.h - Iterate over inst vars ----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines a common pattern for iterating over the variables of an
/// instruction.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINSTVARITER_H
#define SUBZERO_SRC_ICEINSTVARITER_H

/// In Subzero, an Instr may have multiple Ice::Operands, and each Operand can
/// have zero, one, or more Variables.
///
/// We found that a common pattern in Subzero is to iterate over all the
/// Variables in an Instruction. This led to the following pattern being
/// repeated multiple times across the codebase:
///
/// for (Operand Op : Instr.Operands())
///   for (Variable Var : Op.Vars())
///     do_my_thing(Var, Instr)
///
///
/// This code is straightforward, but one may take a couple of seconds to
/// identify what it is doing. We therefore introduce a macroized iterator for
/// hiding this common idiom behind a more explicit interface.
///
/// FOREACH_VAR_IN_INST(Var, Instr) provides this interface. Its first argument
/// needs to be a valid C++ identifier currently undeclared in the current
/// scope; Instr can be any expression yielding a Ice::Inst&&. Even though its
/// definition is ugly, awful, painful-to-read, using it is fairly simple:
///
/// FOREACH_VAR_IN_INST(Var, Instr)
///   do_my_thing(Var, Instr)
///
/// If your loop body contains more than one statement, you can wrap it with a
/// {}, just like any other C++ statement. Note that doing
///
/// FOREACH_VAR_IN_INST(Var0, Instr0)
///   FOREACH_VAR_IN_INST(Var1, Instr1)
///
/// is perfectly safe and legal -- as long as Var0 and Var1 are different
/// identifiers.
///
/// It is sometimes useful to know Var's index in Instr, which can be obtained
/// with
///
/// IndexOfVarInInst(Var)
///
/// Similarly, the current Variable's Operand index can be obtained with
///
/// IndexOfVarOperandInInst(Var).
///
/// And that's pretty much it. Now, if you really hate yourself, keep reading,
/// but beware! The iterator implementation abuses comma operators, for
/// statements, variable initialization and expression evaluations. You have
/// been warned.
///
/// **Implementation details**
///
/// First, let's "break" the two loops into multiple parts:
///
/// for ( Init1; Cond1; Step1 )
///   if ( CondIf )
///     UnreachableThenBody
///   else
///     for ( Init2; Cond2; Step2 )
///
/// The hairiest, scariest, most confusing parts here are Init2 and Cond2, so
/// let's save them for later.
///
///  1) Init1 declares five integer variables:
///      * i          --> outer loop control variable;
///      * Var##Index --> the current variable index
///      * SrcSize    --> how many operands does Instr have?
///      * j          --> the inner loop control variable
///      * NumVars    --> how many variables does the current operand have?
///
///  2) Cond1 and Step1 are your typical for condition and step expressions.
///
///  3) CondIf is where the voodoo starts. We abuse CondIf to declare a local
///  Operand * variable to hold the current operand being evaluated to avoid
///  invoking an Instr::getOperand for each outter loop iteration -- which
///  caused a small performance regression. We initialize the Operand *
///  variable with nullptr, so UnreachableThenBody is trully unreachable, and
///  use the else statement to declare the inner loop. We want to use an else
///  here to prevent code like
///
///      FOREACH_VAR_IN_INST(Var, Instr) {
///      } else {
///      }
///
///  from being legal. We also want to avoid warnings about "dangling else"s.
///
///  4) Init2 is where the voodoo starts. It declares a Variable * local
///  variable name 'Var' (i.e., whatever identifier the first parameter to
///  FOREACH_VAR_IN_INST is), and initializes it with nullptr. Why nullptr?
///  Because as stated above, some operands have zero Variables, and therefore
///  initializing Var = CurrentOperand->Variable(0) would lead to an assertion.
///  Init2 is also required to initialize the control variables used in Cond2,
///  as well as the current Operand * holder,  Therefore, we use the obscure
///  comma operator to initialize Var, and the control variables. The
///  declaration
///
///      Variable *Var = (j = 0, CurrentOperand = Instr.Operand[i],
///                       NumVars = CurrentOperand.NumVars, nullptr)
///
///  achieves that.
///
///  5) Cond2 is where we lose all hopes of having a self-documenting
///  implementation. The stop condition for the inner loop is simply
///
///      j < NumVars
///
///  But there is one more thing we need to do before jumping to the iterator's
///  body: we need to initialize Var with the current variable, but only if the
///  loop has not terminated. So we implemented Cond2 in a way that it would
///  make Var point to the current Variable, but only if there were more
///  variables. So Cond2 became:
///
///      j < NumVars && (Var = CurrentOperand.Var[j])
///
///  which is not quite right. Cond2 would evaluate to false if
///  CurrentOperand.Var[j] == nullptr. Even though that should never happen in
///  Subzero, assuming this is always true is dangerous and could lead to
///  problems in the future. So we abused the comma operator one more time here:
///
///      j < NumVars && ((Var = CurrentOperand.Var[j]), true)
///
///  this expression will evaluate to true if, and only if, j < NumVars.
///
///  6) Step2 increments the inner loop's control variable, as well as the
///  current variable index.
///
/// We use Var -- which should be a valid C++ identifier -- to uniquify names
/// -- e.g., i##Var instead of simply i because we want users to be able to use
/// the iterator for cross-products involving instructions' variables.
#define FOREACH_VAR_IN_INST(Var, Instr)                                        \
  for (SizeT Sz_I##Var##_ = 0, Sz_##Var##Index_ = 0,                           \
             Sz_SrcSize##Var##_ = (Instr).getSrcSize(), Sz_J##Var##_ = 0,      \
             Sz_NumVars##Var##_ = 0, Sz_Foreach_Break = 0;                     \
       !Sz_Foreach_Break && Sz_I##Var##_ < Sz_SrcSize##Var##_; ++Sz_I##Var##_) \
    if (Operand *Sz_Op##Var##_ = nullptr)                                      \
      /*nothing*/;                                                             \
    else                                                                       \
      for (Variable *Var =                                                     \
               (Sz_J##Var##_ = 0,                                              \
               Sz_Op##Var##_ = (Instr).getSrc(Sz_I##Var##_),                   \
               Sz_NumVars##Var##_ = Sz_Op##Var##_->getNumVars(), nullptr);     \
           !Sz_Foreach_Break && Sz_J##Var##_ < Sz_NumVars##Var##_ &&           \
           ((Var = Sz_Op##Var##_->getVar(Sz_J##Var##_)), true);                \
           ++Sz_J##Var##_, ++Sz_##Var##Index_)

#define IsOnlyValidInFOREACH_VAR_IN_INST(V)                                    \
  (static_cast<const SizeT>(Sz_##V##_))
#define IndexOfVarInInst(Var) IsOnlyValidInFOREACH_VAR_IN_INST(Var##Index)
#define IndexOfVarOperandInInst(Var) IsOnlyValidInFOREACH_VAR_IN_INST(I##Var)
#define FOREACH_VAR_IN_INST_BREAK                                              \
  if (true) {                                                                  \
    Sz_Foreach_Break = 1;                                                      \
    continue;                                                                  \
  } else {                                                                     \
  }
#undef OnlyValidIn_FOREACH_VAR_IN_INSTS

#endif // SUBZERO_SRC_ICEINSTVARITER_H
