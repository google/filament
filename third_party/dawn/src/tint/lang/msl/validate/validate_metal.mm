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

// GEN_BUILD:CONDITION(tint_build_is_mac)

#import <Metal/Metal.h>

#include "src/tint/lang/msl/validate/validate.h"

namespace tint::msl::validate {

Result ValidateUsingMetal(const std::string& src_original, MslVersion version) {
    Result result;

    NSError* error = nil;

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        result.output = "MTLCreateSystemDefaultDevice returned null";
        result.failed = true;
        return result;
    }

    std::string src_modified = src_original;
    MTLCompileOptions* compileOptions = [MTLCompileOptions new];
    if (@available(macOS 15.0, iOS 18.0, *)) {
        // Use relaxed math where possible.
        // See crbug.com/425650181
        // The compileOptions.mathMode member is not present on older versions
        // of OSX, and compilation is not protected by the @available check.
        std::string("\n#pragma METAL fp math_mode(relaxed)\n") + src_original;
    } else {
// Silence the warning that fastMathEnabled is deprecated since we cannot remove it until the
// minimum support macOS version is macOS 15.0.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        compileOptions.fastMathEnabled = true;
#pragma clang diagnostic pop
    }
    NSString* source = [NSString stringWithCString:src_modified.c_str()
                                          encoding:NSUTF8StringEncoding];

    switch (version) {
        case MslVersion::kMsl_2_3:
            compileOptions.languageVersion = MTLLanguageVersion2_3;
            break;
        case MslVersion::kMsl_3_2:
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 150000
            if (@available(macOS 15.0, iOS 18.0, *)) {
                compileOptions.languageVersion = MTLLanguageVersion3_2;
                break;
            } else
#endif
            {
                // TODO(crbug.com/434149401): Instead of silently skipping validation, it'd be nice
                // if we could produce a warning here that the requested validation is not
                // happening, in a way that does not break the Tint E2E tests on Dawn CQ.
                return Result{};
            }
    }

    id<MTLLibrary> library = [device newLibraryWithSource:source
                                                  options:compileOptions
                                                    error:&error];
    if (!library) {
        NSString* output = [error localizedDescription];
        result.output = [output UTF8String];
        result.failed = true;
    }

    return result;
}

}  // namespace tint::msl::validate
