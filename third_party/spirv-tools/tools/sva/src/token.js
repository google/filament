// Copyright 2019 Google LLC
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

const TokenType = {
  kEOF: "end of file",
  kError: "error",

  kIdentifier: "identifier",

  kIntegerLiteral: "integer_literal",
  kFloatLiteral: "float_literal",
  kStringLiteral: "string_literal",
  kResultId: "result_id",

  kOp: "Op",
  kEqual: "=",
  kPipe: "|",
};

class Token {
  /**
   * @param {TokenType} type The type of token
   * @param {Integer} line The line number this token was on
   * @param {Any} data Data attached to the token
   * @param {Integer} bits If the type is a float or integer the bit width
   */
  constructor(type, line, data) {
    this.type_ = type;
    this.line_ = line;
    this.data_ = data;
    this.bits_ = 0;
  }

  get type() { return this.type_; }
  get line() { return this.line_; }

  get data() { return this.data_; }
  set data(val) { this.data_ = val; }

  get bits() { return this.bits_; }
  set bits(val) { this.bits_ = val; }
}

export {Token, TokenType};
