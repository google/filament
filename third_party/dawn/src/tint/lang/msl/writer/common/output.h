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

#ifndef SRC_TINT_LANG_MSL_WRITER_COMMON_OUTPUT_H_
#define SRC_TINT_LANG_MSL_WRITER_COMMON_OUTPUT_H_

#include <cstdint>
#include <string>
#include <vector>

namespace tint::msl::writer {

/// The output produced when generating MSL.
struct Output {
    /// Constructor
    Output();

    /// Destructor
    ~Output();

    /// Copy constructor
    Output(const Output&);

    /// Copy assignment
    /// @returns this
    Output& operator=(const Output&);

    /// Workgroup size information
    struct WorkgroupInfo {
        /// The x-component
        uint32_t x = 0;
        /// The y-component
        uint32_t y = 0;
        /// The z-component
        uint32_t z = 0;

        /// A list of dynamic workgroup allocations.
        /// Each entry in the vector is the size of the workgroup allocation that
        /// should be created for that index.
        std::vector<uint32_t> allocations;

        /// The needed workgroup storage size
        size_t storage_size = 0;
    };

    /// The generated MSL.
    std::string msl = "";

    /// True if the shader needs a UBO of buffer sizes.
    bool needs_storage_buffer_sizes = false;

    /// True if the generated shader uses the invariant attribute.
    bool has_invariant_attribute = false;

    /// The workgroup size information, if the entry point was a compute shader
    WorkgroupInfo workgroup_info{};
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_COMMON_OUTPUT_H_
