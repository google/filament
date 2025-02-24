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

#include "src/tint/lang/spirv/reader/ast_parser/construct.h"

namespace tint::spirv::reader::ast_parser {

Construct::Construct(const Construct* the_parent,
                     int the_depth,
                     Kind the_kind,
                     uint32_t the_begin_id,
                     uint32_t the_end_id,
                     uint32_t the_begin_pos,
                     uint32_t the_end_pos,
                     uint32_t the_scope_end_pos)
    : parent(the_parent),
      enclosing_loop(
          // Compute the enclosing loop construct. Doing this in the
          // constructor member list lets us make the member const.
          // Compare parent depth because loop and continue are siblings and
          // it's incidental which will appear on the stack first.
          the_kind == kLoop
              ? this
              : ((parent && parent->depth < the_depth) ? parent->enclosing_loop : nullptr)),
      enclosing_continue(
          // Compute the enclosing continue construct. Doing this in the
          // constructor member list lets us make the member const.
          // Compare parent depth because loop and continue are siblings and
          // it's incidental which will appear on the stack first.
          the_kind == kContinue
              ? this
              : ((parent && parent->depth < the_depth) ? parent->enclosing_continue : nullptr)),
      enclosing_loop_or_continue_or_switch(
          // Compute the enclosing loop or continue or switch construct.
          // Doing this in the constructor member list lets us make the
          // member const.
          // Compare parent depth because loop and continue are siblings and
          // it's incidental which will appear on the stack first.
          (the_kind == kLoop || the_kind == kContinue || the_kind == kSwitchSelection)
              ? this
              : ((parent && parent->depth < the_depth)
                     ? parent->enclosing_loop_or_continue_or_switch
                     : nullptr)),
      depth(the_depth),
      kind(the_kind),
      begin_id(the_begin_id),
      end_id(the_end_id),
      begin_pos(the_begin_pos),
      end_pos(the_end_pos),
      scope_end_pos(the_scope_end_pos) {}

}  // namespace tint::spirv::reader::ast_parser
