// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/Error.h"

#include "dawn/native/EnumClassBitmasks.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

void IgnoreErrors(MaybeError maybeError) {
    if (maybeError.IsError()) {
        std::unique_ptr<ErrorData> errorData = maybeError.AcquireError();
        // During shutdown and destruction, device lost errors can be ignored.
        // We can also ignore other unexpected internal errors on shut down and treat it as
        // device lost so that we can continue with destruction.
        DAWN_ASSERT(errorData->GetType() == InternalErrorType::DeviceLost ||
                    errorData->GetType() == InternalErrorType::Internal);
    }
}

wgpu::ErrorType ToWGPUErrorType(InternalErrorType type) {
    switch (type) {
        case InternalErrorType::Validation:
            return wgpu::ErrorType::Validation;
        case InternalErrorType::OutOfMemory:
            return wgpu::ErrorType::OutOfMemory;
        case InternalErrorType::Internal:
            return wgpu::ErrorType::Internal;

        default:
            return wgpu::ErrorType::Unknown;
    }
}

InternalErrorType FromWGPUErrorType(wgpu::ErrorType type) {
    switch (type) {
        case wgpu::ErrorType::Validation:
            return InternalErrorType::Validation;
        case wgpu::ErrorType::OutOfMemory:
            return InternalErrorType::OutOfMemory;
        default:
            return InternalErrorType::Internal;
    }
}

absl::FormatConvertResult<absl::FormatConversionCharSet::kString |
                          absl::FormatConversionCharSet::kIntegral>
AbslFormatConvert(InternalErrorType value,
                  const absl::FormatConversionSpec& spec,
                  absl::FormatSink* s) {
    if (spec.conversion_char() == absl::FormatConversionChar::s) {
        if (!static_cast<bool>(value)) {
            s->Append("None");
            return {true};
        }

        bool moreThanOneBit = !HasZeroOrOneBits(value);
        if (moreThanOneBit) {
            s->Append("(");
        }

        bool first = true;
        if (value & InternalErrorType::Validation) {
            if (!first) {
                s->Append("|");
            }
            first = false;
            s->Append("Validation");
            value &= ~InternalErrorType::Validation;
        }
        if (value & InternalErrorType::DeviceLost) {
            if (!first) {
                s->Append("|");
            }
            first = false;
            s->Append("DeviceLost");
            value &= ~InternalErrorType::DeviceLost;
        }
        if (value & InternalErrorType::Internal) {
            if (!first) {
                s->Append("|");
            }
            first = false;
            s->Append("Internal");
            value &= ~InternalErrorType::Internal;
        }
        if (value & InternalErrorType::OutOfMemory) {
            if (!first) {
                s->Append("|");
            }
            first = false;
            s->Append("OutOfMemory");
            value &= ~InternalErrorType::OutOfMemory;
        }

        if (moreThanOneBit) {
            s->Append(")");
        }
    } else {
        s->Append(absl::StrFormat(
            "%u", static_cast<typename std::underlying_type<InternalErrorType>::type>(value)));
    }
    return {true};
}

}  // namespace dawn::native
