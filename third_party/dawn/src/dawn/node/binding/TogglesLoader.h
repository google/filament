// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NODE_BINDING_TOGGLESLOADER_H_
#define SRC_DAWN_NODE_BINDING_TOGGLESLOADER_H_

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

#include "src/dawn/node/binding/Flags.h"

namespace wgpu::binding {
class TogglesLoader {
  public:
    // Constructor, reading toggles from the "enable-dawn-features"
    // and "disable-dawn-features" flags.
    explicit TogglesLoader(const Flags& flags);

    // Returns a DawnTogglesDescriptor populated with toggles
    // read at constructor time. It is only valid for the lifetime
    // of this TogglesLoader object.
    DawnTogglesDescriptor GetDescriptor();

  private:
    // Ban copy-assignment and copy-construction. The compiler will
    // create implicitly-declared move-constructor and move-assignment
    // implementations as needed.
    TogglesLoader(const TogglesLoader& other) = delete;
    TogglesLoader& operator=(const TogglesLoader&) = delete;

    // DawnTogglesDescriptor::enabledToggles and disabledToggles are vectors
    // of 'const char*', so keep local copies of the strings, and don't allow
    // them to be relocated.
    std::vector<std::string> enabledTogglesStrings_;
    std::vector<std::string> disabledTogglesStrings_;
    std::vector<const char*> enabledToggles_;
    std::vector<const char*> disabledToggles_;
};
}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_TOGGLESLOADER_H_
