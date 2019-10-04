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

import Parser from "./parser.js";
import Lexer from "./lexer.js";
import Assembler from "./assembler.js";

import grammar from "./spirv.data.js";

export default class SVA {
  /**
   * Attempts to convert |input| SPIR-V assembly into SPIR-V binary.
   *
   * @param {String} the input string containing the assembly
   * @return {Uint32Array|string} returns a Uint32Array containing the binary
   *                             SPIR-V or a string on error.
   */
  static assemble(input) {
    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    if (ast === undefined)
      return p.error;

    let a = new Assembler(ast);
    return a.assemble();
  }
}
