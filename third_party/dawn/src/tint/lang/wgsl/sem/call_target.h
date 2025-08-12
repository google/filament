// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_WGSL_SEM_CALL_TARGET_H_
#define SRC_TINT_LANG_WGSL_SEM_CALL_TARGET_H_

#include <vector>

#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/wgsl/sem/node.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/math/hash.h"

namespace tint::sem {

/// CallTargetSignature holds the return type and parameters for a call target
struct CallTargetSignature {
    /// Constructor
    CallTargetSignature();

    /// Constructor
    /// @param ret_ty the call target return type
    /// @param params the call target parameters
    CallTargetSignature(const core::type::Type* ret_ty, VectorRef<const Parameter*> params);

    /// Copy constructor
    CallTargetSignature(const CallTargetSignature&);

    /// Destructor
    ~CallTargetSignature();

    /// @returns the hash code of the CallTargetSignature
    tint::HashCode HashCode() const;

    /// Equality operator
    /// @param other the signature to compare this to
    /// @returns true if this signature is equal to other
    bool operator==(const CallTargetSignature& other) const;

    /// @param usage the parameter usage to find
    /// @returns the index of the parameter with the given usage, or -1 if no
    /// parameter with the given usage exists.
    int IndexOf(core::ParameterUsage usage) const;

    /// The type of the call target return value
    const core::type::Type* return_type = nullptr;

    /// The parameters of the call target
    tint::Vector<const sem::Parameter*, 8> parameters;
};

/// CallTarget is the base for callable functions, builtins, value constructors and value
/// conversions.
class CallTarget : public Castable<CallTarget, Node> {
  public:
    /// Constructor
    /// @param stage the earliest evaluation stage for a call to this target
    /// @param must_use the result of the call target must be used, i.e. it cannot be used as a call
    /// statement.
    CallTarget(core::EvaluationStage stage, bool must_use);

    /// Constructor
    /// @param return_type the return type of the call target
    /// @param parameters the parameters for the call target
    /// @param stage the earliest evaluation stage for a call to this target
    /// @param must_use the result of the call target must be used, i.e. it cannot be used as a call
    /// statement.
    CallTarget(const core::type::Type* return_type,
               VectorRef<Parameter*> parameters,
               core::EvaluationStage stage,
               bool must_use);

    /// Copy constructor
    CallTarget(const CallTarget&);

    /// Destructor
    ~CallTarget() override;

    /// Sets the call target's return type
    /// @param ty the parameter
    void SetReturnType(const core::type::Type* ty) { signature_.return_type = ty; }

    /// @return the return type of the call target
    const core::type::Type* ReturnType() const { return signature_.return_type; }

    /// Adds a parameter to the call target
    /// @param parameter the parameter
    void AddParameter(Parameter* parameter) {
        parameter->SetOwner(this);
        signature_.parameters.Push(parameter);
    }

    /// @return the parameters of the call target
    auto& Parameters() const { return signature_.parameters; }

    /// @return the signature of the call target
    const CallTargetSignature& Signature() const { return signature_; }

    /// @return the earliest evaluation stage for a call to this target
    core::EvaluationStage Stage() const { return stage_; }

    /// @returns true if the result of the call target must be used, i.e. it cannot be used as a
    /// call statement.
    bool MustUse() const { return must_use_; }

  private:
    CallTargetSignature signature_;
    core::EvaluationStage stage_;
    const bool must_use_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_CALL_TARGET_H_
