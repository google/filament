// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ENTRY_POINT_INFO_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ENTRY_POINT_INFO_H_

#include <string>

#include "src/tint/lang/wgsl/ast/pipeline_stage.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::spirv::reader::ast_parser {

/// The size of an integer-coordinate grid, in the x, y, and z dimensions.
struct GridSize {
    /// x value
    uint32_t x = 0;
    /// y value
    uint32_t y = 0;
    /// z value
    uint32_t z = 0;
};

/// Entry point information for a function
struct EntryPointInfo {
    /// Constructor.
    /// @param the_name the name of the entry point
    /// @param the_stage the pipeline stage
    /// @param the_owns_inner_implementation if true, this entry point is
    /// responsible for generating the inner implementation function.
    /// @param the_inner_name the name of the inner implementation function of the
    /// entry point
    /// @param the_inputs list of IDs for Input variables used by the shader
    /// @param the_outputs list of IDs for Output variables used by the shader
    /// @param the_wg_size the workgroup_size, for a compute shader
    EntryPointInfo(std::string the_name,
                   ast::PipelineStage the_stage,
                   bool the_owns_inner_implementation,
                   std::string the_inner_name,
                   VectorRef<uint32_t> the_inputs,
                   VectorRef<uint32_t> the_outputs,
                   GridSize the_wg_size);
    /// Copy constructor
    /// @param other the other entry point info to be built from
    EntryPointInfo(const EntryPointInfo& other);
    /// Destructor
    ~EntryPointInfo();

    /// The entry point name.
    /// In the WGSL output, this function will have pipeline inputs and outputs
    /// as parameters. This function will store them into Private variables,
    /// and then call the "inner" function, named by the next memeber.
    /// Then outputs are copied from the private variables to the return value.
    std::string name;
    /// The entry point stage
    ast::PipelineStage stage = ast::PipelineStage::kNone;

    /// True when this entry point is responsible for generating the
    /// inner implementation function.  False when this is the second entry
    /// point encountered for the same function in SPIR-V. It's unusual, but
    /// possible for the same function to be the implementation for multiple
    /// entry points.
    bool owns_inner_implementation;
    /// The name of the inner implementation function of the entry point.
    std::string inner_name;
    /// IDs of pipeline input variables, sorted and without duplicates.
    tint::Vector<uint32_t, 8> inputs;
    /// IDs of pipeline output variables, sorted and without duplicates.
    tint::Vector<uint32_t, 8> outputs;

    /// If this is a compute shader, this is the workgroup size in the x, y,
    /// and z dimensions set via LocalSize, or via the composite value
    /// decorated as the WorkgroupSize BuiltIn.  The WorkgroupSize builtin
    /// takes priority.
    GridSize workgroup_size;
};

}  // namespace tint::spirv::reader::ast_parser

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ENTRY_POINT_INFO_H_
