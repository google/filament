//===- llvm/PassInfo.h - Pass Info class ------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines and implements the PassInfo class.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_PASSINFO_H
#define LLVM_PASSINFO_H

#include <cassert>
#include <vector>

namespace llvm {

class Pass;
class TargetMachine;

//===---------------------------------------------------------------------------
/// PassInfo class - An instance of this class exists for every pass known by
/// the system, and can be obtained from a live Pass by calling its
/// getPassInfo() method.  These objects are set up by the RegisterPass<>
/// template.
///
class PassInfo {
public:
  typedef Pass* (*NormalCtor_t)();
  typedef Pass *(*TargetMachineCtor_t)(TargetMachine *);

private:
  const char      *const PassName;     // Nice name for Pass
  const char      *const PassArgument; // Command Line argument to run this pass
  const void *PassID;      
  const bool IsCFGOnlyPass;            // Pass only looks at the CFG.
  const bool IsAnalysis;               // True if an analysis pass.
  const bool IsAnalysisGroup;          // True if an analysis group.
  std::vector<const PassInfo*> ItfImpl;// Interfaces implemented by this pass

  NormalCtor_t NormalCtor;
  TargetMachineCtor_t TargetMachineCtor;

public:
  /// PassInfo ctor - Do not call this directly, this should only be invoked
  /// through RegisterPass.
  PassInfo(const char *name, const char *arg, const void *pi,
           NormalCtor_t normal, bool isCFGOnly, bool is_analysis,
           TargetMachineCtor_t machine = nullptr)
    : PassName(name), PassArgument(arg), PassID(pi), 
      IsCFGOnlyPass(isCFGOnly), 
      IsAnalysis(is_analysis), IsAnalysisGroup(false), NormalCtor(normal),
      TargetMachineCtor(machine) {}
  /// PassInfo ctor - Do not call this directly, this should only be invoked
  /// through RegisterPass. This version is for use by analysis groups; it
  /// does not auto-register the pass.
  PassInfo(const char *name, const void *pi)
    : PassName(name), PassArgument(""), PassID(pi), 
      IsCFGOnlyPass(false), 
      IsAnalysis(false), IsAnalysisGroup(true), NormalCtor(nullptr),
      TargetMachineCtor(nullptr) {}

  /// getPassName - Return the friendly name for the pass, never returns null
  ///
  StringRef getPassName() const { return PassName; }

  /// getPassArgument - Return the command line option that may be passed to
  /// 'opt' that will cause this pass to be run.  This will return null if there
  /// is no argument.
  ///
  const char *getPassArgument() const { return PassArgument; }

  /// getTypeInfo - Return the id object for the pass...
  /// TODO : Rename
  const void *getTypeInfo() const { return PassID; }

  /// Return true if this PassID implements the specified ID pointer.
  bool isPassID(const void *IDPtr) const {
    return PassID == IDPtr;
  }
  
  /// isAnalysisGroup - Return true if this is an analysis group, not a normal
  /// pass.
  ///
  bool isAnalysisGroup() const { return IsAnalysisGroup; }
  bool isAnalysis() const { return IsAnalysis; }

  /// isCFGOnlyPass - return true if this pass only looks at the CFG for the
  /// function.
  bool isCFGOnlyPass() const { return IsCFGOnlyPass; }
  
  /// getNormalCtor - Return a pointer to a function, that when called, creates
  /// an instance of the pass and returns it.  This pointer may be null if there
  /// is no default constructor for the pass.
  ///
  NormalCtor_t getNormalCtor() const {
    return NormalCtor;
  }
  void setNormalCtor(NormalCtor_t Ctor) {
    NormalCtor = Ctor;
  }

  /// getTargetMachineCtor - Return a pointer to a function, that when called
  /// with a TargetMachine, creates an instance of the pass and returns it.
  /// This pointer may be null if there is no constructor with a TargetMachine
  /// for the pass.
  ///
  TargetMachineCtor_t getTargetMachineCtor() const { return TargetMachineCtor; }
  void setTargetMachineCtor(TargetMachineCtor_t Ctor) {
    TargetMachineCtor = Ctor;
  }

  /// createPass() - Use this method to create an instance of this pass.
  Pass *createPass() const {
    assert((!isAnalysisGroup() || NormalCtor) &&
           "No default implementation found for analysis group!");
    assert(NormalCtor &&
           "Cannot call createPass on PassInfo without default ctor!");
    return NormalCtor();
  }

  /// addInterfaceImplemented - This method is called when this pass is
  /// registered as a member of an analysis group with the RegisterAnalysisGroup
  /// template.
  ///
  void addInterfaceImplemented(const PassInfo *ItfPI) {
    ItfImpl.push_back(ItfPI);
  }

  /// getInterfacesImplemented - Return a list of all of the analysis group
  /// interfaces implemented by this pass.
  ///
  const std::vector<const PassInfo*> &getInterfacesImplemented() const {
    return ItfImpl;
  }

private:
  void operator=(const PassInfo &) = delete;
  PassInfo(const PassInfo &) = delete;
};

}

#endif
