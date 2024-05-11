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

import { assert } from "chai";
import Lexer from "./lexer";
import Parser from "./parser";
import grammar from "./spirv.data.js";
import Assembler from "./assembler";

describe("assembler", () => {
  it("generates SPIR-V magic number", () => {
    let input = `; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 6
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 440
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd`;
    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast, p.error);

    let a = new Assembler(ast);
    let res = a.assemble();
    assert.equal(res[0], 0x07230203);
  });

  it("assembles enumerant params", () => {
    let input = "OpExecutionMode %main LocalSize 2 3 4";

    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast, p.error);

    let a = new Assembler(ast);
    let res = a.assemble();

    assert.lengthOf(res, 11);
    assert.equal(res[5], (6 /* word count */ << 16) | 16 /* opcode */);
    assert.equal(res[6], 1 /* %main */);
    assert.equal(res[7], 17 /* LocalSize */);
    assert.equal(res[8], 2);
    assert.equal(res[9], 3);
    assert.equal(res[10], 4);
  });

  it("assembles float 32 values", () => {
    let input = `%float = OpTypeFloat 32
                 %float1 = OpConstant %float 0.400000006`;
    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast, p.error);

    let a = new Assembler(ast);
    let res = a.assemble();

    assert.lengthOf(res, 12);
    assert.equal(res[8], (4 /* word count */ << 16) | 43 /* opcode */);
    assert.equal(res[9], 1 /* %float */);
    assert.equal(res[10], 2 /* %float */);
    assert.equal(res[11], 0x3ecccccd /* 0.400000006 */);
  });

  describe("strings", () => {
    it("assembles 'abcd'", () => {
      let input = `OpName %mains "abcd"`;
      let l = new Lexer(input);
      let p = new Parser(grammar, l);

      let ast = p.parse();
      assert.exists(ast, p.error);

      let a = new Assembler(ast);
      let res = a.assemble();

      assert.lengthOf(res, 9);
      assert.equal(res[5], (4 /* word count */ << 16) | 5 /* opcode */);
      assert.equal(res[6], 1 /* %mains */);
      assert.equal(res[7], 0x64636261 /* food */);
      assert.equal(res[8], 0x00000000 /* null byte */);
    });

    it("assembles 'abcde'", () => {
      let input = `OpName %mains "abcde"`;
      let l = new Lexer(input);
      let p = new Parser(grammar, l);

      let ast = p.parse();
      assert.exists(ast, p.error);

      let a = new Assembler(ast);
      let res = a.assemble();

      assert.lengthOf(res, 9);
      assert.equal(res[5], (4 /* word count */ << 16) | 5 /* opcode */);
      assert.equal(res[6], 1 /* %mains */);
      assert.equal(res[7], 0x64636261 /* abcd */);
      assert.equal(res[8], 0x00000065 /* e */);
    });

    it("assembles 'abcdef'", () => {
      let input = `OpName %mains "abcdef"`;
      let l = new Lexer(input);
      let p = new Parser(grammar, l);

      let ast = p.parse();
      assert.exists(ast, p.error);

      let a = new Assembler(ast);
      let res = a.assemble();

      assert.lengthOf(res, 9);
      assert.equal(res[5], (4 /* word count */ << 16) | 5 /* opcode */);
      assert.equal(res[6], 1 /* %mains */);
      assert.equal(res[7], 0x64636261 /* abcd */);
      assert.equal(res[8], 0x00006665 /* ef */);
    });

    it("assembles 'abcdefg'", () => {
      let input = `OpName %mains "abcdefg"`;
      let l = new Lexer(input);
      let p = new Parser(grammar, l);

      let ast = p.parse();
      assert.exists(ast, p.error);

      let a = new Assembler(ast);
      let res = a.assemble();

      assert.lengthOf(res, 9);
      assert.equal(res[5], (4 /* word count */ << 16) | 5 /* opcode */);
      assert.equal(res[6], 1 /* %mains */);
      assert.equal(res[7], 0x64636261 /* abcd */);
      assert.equal(res[8], 0x00676665 /* efg */);
    });
  });
});
