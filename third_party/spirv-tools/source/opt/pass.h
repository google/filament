// Copyright (c) 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SOURCE_OPT_PASS_H_
#define SOURCE_OPT_PASS_H_

#include <algorithm>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "source/opt/basic_block.h"
#include "source/opt/def_use_manager.h"
#include "source/opt/ir_context.h"
#include "source/opt/module.h"
#include "spirv-tools/libspirv.hpp"

namespace spvtools {
namespace opt {

// Abstract class of a pass. All passes should implement this abstract class
// and all analysis and transformation is done via the Process() method.
class Pass {
 public:
  // The status of processing a module using a pass.
  //
  // The numbers for the cases are assigned to make sure that Failure & anything
  // is Failure, SuccessWithChange & any success is SuccessWithChange.
  enum class Status {
    Failure = 0x00,
    SuccessWithChange = 0x10,
    SuccessWithoutChange = 0x11,
  };

  using ProcessFunction = std::function<bool(Function*)>;

  // Destructs the pass.
  virtual ~Pass() = default;

  // Returns a descriptive name for this pass.
  //
  // NOTE: When deriving a new pass class, make sure you make the name
  // compatible with the corresponding spirv-opt command-line flag. For example,
  // if you add the flag --my-pass to spirv-opt, make this function return
  // "my-pass" (no leading hyphens).
  virtual const char* name() const = 0;

  // Sets the message consumer to the given |consumer|. |consumer| which will be
  // invoked every time there is a message to be communicated to the outside.
  void SetMessageConsumer(MessageConsumer c) { consumer_ = std::move(c); }

  // Returns the reference to the message consumer for this pass.
  const MessageConsumer& consumer() const { return consumer_; }

  // Returns the def-use manager used for this pass. TODO(dnovillo): This should
  // be handled by the pass manager.
  analysis::DefUseManager* get_def_use_mgr() const {
    return context()->get_def_use_mgr();
  }

  analysis::DecorationManager* get_decoration_mgr() const {
    return context()->get_decoration_mgr();
  }

  FeatureManager* get_feature_mgr() const {
    return context()->get_feature_mgr();
  }

  // Returns a pointer to the current module for this pass.
  Module* get_module() const { return context_->module(); }

  // Sets the pointer to the current context for this pass.
  void SetContextForTesting(IRContext* ctx) { context_ = ctx; }

  // Returns a pointer to the current context for this pass.
  IRContext* context() const { return context_; }

  // Returns a pointer to the CFG for current module.
  CFG* cfg() const { return context()->cfg(); }

  // Add to |todo| all ids of functions called in |func|.
  void AddCalls(Function* func, std::queue<uint32_t>* todo);

  // Applies |pfn| to every function in the call trees that are rooted at the
  // entry points.  Returns true if any call |pfn| returns true.  By convention
  // |pfn| should return true if it modified the module.
  bool ProcessEntryPointCallTree(ProcessFunction& pfn, Module* module);

  // Applies |pfn| to every function in the call trees rooted at the entry
  // points and exported functions.  Returns true if any call |pfn| returns
  // true.  By convention |pfn| should return true if it modified the module.
  bool ProcessReachableCallTree(ProcessFunction& pfn, IRContext* irContext);

  // Applies |pfn| to every function in the call trees rooted at the elements of
  // |roots|.  Returns true if any call to |pfn| returns true.  By convention
  // |pfn| should return true if it modified the module.  After returning
  // |roots| will be empty.
  bool ProcessCallTreeFromRoots(
      ProcessFunction& pfn,
      const std::unordered_map<uint32_t, Function*>& id2function,
      std::queue<uint32_t>* roots);

  // Run the pass on the given |module|. Returns Status::Failure if errors occur
  // when processing. Returns the corresponding Status::Success if processing is
  // successful to indicate whether changes are made to the module.  If there
  // were any changes it will also invalidate the analyses in the IRContext
  // that are not preserved.
  //
  // It is an error if |Run| is called twice with the same instance of the pass.
  // If this happens the return value will be |Failure|.
  Status Run(IRContext* ctx);

  // Returns the set of analyses that the pass is guaranteed to preserve.
  virtual IRContext::Analysis GetPreservedAnalyses() {
    return IRContext::kAnalysisNone;
  }

  // Return type id for |ptrInst|'s pointee
  uint32_t GetPointeeTypeId(const Instruction* ptrInst) const;

 protected:
  // Constructs a new pass.
  //
  // The constructed instance will have an empty message consumer, which just
  // ignores all messages from the library. Use SetMessageConsumer() to supply
  // one if messages are of concern.
  Pass();

  // Processes the given |module|. Returns Status::Failure if errors occur when
  // processing. Returns the corresponding Status::Success if processing is
  // succesful to indicate whether changes are made to the module.
  virtual Status Process() = 0;

  // Return the next available SSA id and increment it.
  uint32_t TakeNextId() { return context_->TakeNextId(); }

 private:
  MessageConsumer consumer_;  // Message consumer.

  // The context that this pass belongs to.
  IRContext* context_;

  // An instance of a pass can only be run once because it is too hard to
  // enforce proper resetting of internal state for each instance.  This member
  // is used to check that we do not run the same instance twice.
  bool already_run_;
};

}  // namespace opt
}  // namespace spvtools

#endif  // SOURCE_OPT_PASS_H_
