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

describe("parser", () => {
  it("parses an opcode", () => {
    let input = "OpKill";
    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast);
    assert.lengthOf(ast.instructions(), 1);

    let inst = ast.instruction(0);
    assert.equal(inst.name(), "OpKill");
    assert.equal(inst.opcode(), 252);
    assert.lengthOf(inst.operands, 0);
  });

  it("parses an opcode with an identifier", () => {
    let input = "OpCapability Shader";
    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast, p.error);
    assert.lengthOf(ast.instructions(), 1);

    let inst = ast.instruction(0);
    assert.equal(inst.name(), "OpCapability");
    assert.equal(inst.opcode(), 17);
    assert.lengthOf(inst.operands(), 1);

    let op = inst.operand(0);
    assert.equal(op.name(), "Shader");
    assert.equal(op.type(), "ValueEnum");
    assert.equal(op.value(), 1);
  });

  it("parses an opcode with a result", () => {
    let input = "%void = OpTypeVoid";
    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast);
    assert.lengthOf(ast.instructions(), 1);

    let inst = ast.instruction(0);
    assert.equal(inst.name(), "OpTypeVoid");
    assert.equal(inst.opcode(), 19);
    assert.lengthOf(inst.operands(), 1);

    let op = inst.operand(0);
    assert.equal(op.name(), "void");
    assert.equal(op.value(), 1);
  });

  it("sets module bounds based on numeric result", () => {
    let input = "%3 = OpTypeVoid";

    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast);
    assert.equal(ast.getId("next"), 4);
  });

  it("returns the same value for a named result_id", () => {
    let input = "%3 = OpTypeFunction %int %int";

    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast);
    assert.lengthOf(ast.instructions(), 1);

    let inst = ast.instruction(0);
    let op1 = inst.operand(1);
    assert.equal(op1.name(), "int");
    assert.equal(op1.value(), 4);

    let op2 = inst.operand(2);
    assert.equal(op2.name(), "int");
    assert.equal(op2.value(), 4);
  });

  it("parses an opcode with a string", () => {
    let input = "OpEntryPoint Fragment %main \"main\"";

    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast);
    assert.lengthOf(ast.instructions(), 1);

    let inst = ast.instruction(0);
    let op = inst.operand(2);
    assert.equal(op.name(), "main");
    assert.equal(op.value(), "main");
  });

  describe("numerics", () => {
    describe("integers", () => {
      it("parses an opcode with an integer", () => {
        let input = "OpSource GLSL 440";

        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast);
        assert.lengthOf(ast.instructions(), 1);

        let inst = ast.instruction(0);
        let op0 = inst.operand(0);
        assert.equal(op0.name(), "GLSL");
        assert.equal(op0.type(), "ValueEnum");
        assert.equal(op0.value(), 2);

        let op1 = inst.operand(1);
        assert.equal(op1.name(), "440");
        assert.equal(op1.value(), 440);
      });

      it("parses an opcode with a hex integer", () => {
        let input = "OpSource GLSL 0x440";

        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast);
        assert.lengthOf(ast.instructions(), 1);

        let inst = ast.instruction(0);
        let op0 = inst.operand(0);
        assert.equal(op0.name(), "GLSL");
        assert.equal(op0.type(), "ValueEnum");
        assert.equal(op0.value(), 2);

        let op1 = inst.operand(1);
        assert.equal(op1.name(), "1088");
        assert.equal(op1.value(), 0x440);
      });

      it.skip("parses immediate integers", () => {
        // TODO(dsinclair): Support or skip?
      });
    });

    describe("floats", () => {
      it("parses floats", () => {
        let input = `%float = OpTypeFloat 32
                     %float1 = OpConstant %float 0.400000006`;

        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast, p.error);
        assert.lengthOf(ast.instructions(), 2);

        let inst = ast.instruction(1);
        let op2 = inst.operand(2);
        assert.equal(op2.value(), 0.400000006);
      });

      // TODO(dsinclair): Make hex encoded floats parse ...
      it.skip("parses hex floats", () => {
        let input = `%float = OpTypeFloat 32
                     %nfloat = OpConstant %float -0.4p+2
                     %pfloat = OpConstant %float 0.4p-2
                     %inf = OpConstant %float32 0x1p+128
                     %neginf = OpConstant %float32 -0x1p+128
                     %aNaN = OpConstant %float32 0x1.8p+128
                     %moreNaN = OpConstant %float32 -0x1.0002p+128`;

        let results = [-40.0, .004, 0x00000, 0x00000, 0x7fc00000, 0xff800100];
        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast, p.error);
        assert.lengthOf(ast.instructions(), 7);

        for (const idx in results) {
          let inst = ast.instruction(idx);
          let op2 = inst.operand(2);
          assert.equal(op2.value(), results[idx]);
        }
      });

      it("parses a float that looks like an int", () => {
        let input = `%float = OpTypeFloat 32
                     %float1 = OpConstant %float 1`;

        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast, p.error);
        assert.lengthOf(ast.instructions(), 2);

        let inst = ast.instruction(1);
        let op2 = inst.operand(2);
        assert.equal(op2.value(), 1);
        assert.equal(op2.type(), "float");
      });
    });
  });

  describe("enums", () => {
    it("parses enum values", () => {
      let input = `%1 = OpTypeFloat 32
  %30 = OpImageSampleExplicitLod %1 %20 %18 Grad|ConstOffset %22 %24 %29`;

      let vals = [{val: 1, name: "1"},
        {val: 30, name: "30"},
        {val: 20, name: "20"},
        {val: 18, name: "18"},
        {val: 12, name: "Grad|ConstOffset"}];

      let l = new Lexer(input);
      let p = new Parser(grammar, l);

      let ast = p.parse();
      assert.exists(ast, p.error);
      assert.lengthOf(ast.instructions(), 2);

      let inst = ast.instruction(1);
      for (let idx in vals) {
        let op = inst.operand(idx);
        assert.equal(op.name(), vals[idx].name);
        assert.equal(op.value(), vals[idx].val);
      }

      // BitEnum
      let params = inst.operand(4).params();
      assert.lengthOf(params, 3);
      assert.equal(params[0].name(), "22");
      assert.equal(params[0].value(), 22);
      assert.equal(params[1].name(), "24");
      assert.equal(params[1].value(), 24);
      assert.equal(params[2].name(), "29");
      assert.equal(params[2].value(), 29);
    });

    it("parses enumerants with parameters", () => {
      let input ="OpExecutionMode %main LocalSize 2 3 4";

      let l = new Lexer(input);
      let p = new Parser(grammar, l);

      let ast = p.parse();
      assert.exists(ast, p.error);
      assert.lengthOf(ast.instructions(), 1);

      let inst = ast.instruction(0);
      assert.equal(inst.name(), "OpExecutionMode");
      assert.lengthOf(inst.operands(), 2);
      assert.equal(inst.operand(0).name(), "main");
      assert.equal(inst.operand(1).name(), "LocalSize");

      let params = inst.operand(1).params();
      assert.lengthOf(params, 3);
      assert.equal(params[0].name(), "2");
      assert.equal(params[1].name(), "3");
      assert.equal(params[2].name(), "4");
    });
  });

  it("parses result into second operand if needed", () => {
    let input = `%int = OpTypeInt 32 1
                 %int_3 = OpConstant %int 3`;
    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast);
    assert.lengthOf(ast.instructions(), 2);

    let inst = ast.instruction(1);
    assert.equal(inst.name(), "OpConstant");
    assert.equal(inst.opcode(), 43);
    assert.lengthOf(inst.operands(), 3);

    let op0 = inst.operand(0);
    assert.equal(op0.name(), "int");
    assert.equal(op0.value(), 1);

    let op1 = inst.operand(1);
    assert.equal(op1.name(), "int_3");
    assert.equal(op1.value(), 2);

    let op2 = inst.operand(2);
    assert.equal(op2.name(), "3");
    assert.equal(op2.value(), 3);
  });

  describe("quantifiers", () => {
    describe("?", () => {
      it("skips if missing", () => {
        let input = `OpImageWrite %1 %2 %3
OpKill`;
        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast);
        assert.lengthOf(ast.instructions(), 2);

        let inst = ast.instruction(0);
        assert.equal(inst.name(), "OpImageWrite");
        assert.lengthOf(inst.operands(), 3);
      });

      it("skips if missing at EOF", () => {
        let input = "OpImageWrite %1 %2 %3";
        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast);
        assert.lengthOf(ast.instructions(), 1);

        let inst = ast.instruction(0);
        assert.equal(inst.name(), "OpImageWrite");
        assert.lengthOf(inst.operands(), 3);
      });

      it("extracts if available", () => {
        let input = `OpImageWrite %1 %2 %3 ConstOffset %2
OpKill`;
        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast);
        assert.lengthOf(ast.instructions(), 2);

        let inst = ast.instruction(0);
        assert.equal(inst.name(), "OpImageWrite");
        assert.lengthOf(inst.operands(), 4);
        assert.equal(inst.operand(3).name(), "ConstOffset");
      });
    });

    describe("*", () => {
      it("skips if missing", () => {
        let input = `OpEntryPoint Fragment %main "main"
OpKill`;

        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast);
        assert.lengthOf(ast.instructions(), 2);

        let inst = ast.instruction(0);
        assert.equal(inst.name(), "OpEntryPoint");
        assert.lengthOf(inst.operands(), 3);
        assert.equal(inst.operand(2).name(), "main");
      });

      it("extracts one if available", () => {
        let input = `OpEntryPoint Fragment %main "main" %2
OpKill`;

        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast);
        assert.lengthOf(ast.instructions(), 2);

        let inst = ast.instruction(0);
        assert.equal(inst.name(), "OpEntryPoint");
        assert.lengthOf(inst.operands(), 4);
        assert.equal(inst.operand(3).name(), "2");
      });

      it("extracts multiple if available", () => {
        let input = `OpEntryPoint Fragment %main "main" %2 %3 %4 %5
OpKill`;

        let l = new Lexer(input);
        let p = new Parser(grammar, l);

        let ast = p.parse();
        assert.exists(ast);
        assert.lengthOf(ast.instructions(), 2);

        let inst = ast.instruction(0);
        assert.equal(inst.name(), "OpEntryPoint");
        assert.lengthOf(inst.operands(), 7);
        assert.equal(inst.operand(3).name(), "2");
        assert.equal(inst.operand(4).name(), "3");
        assert.equal(inst.operand(5).name(), "4");
        assert.equal(inst.operand(6).name(), "5");
      });
    });
  });

  describe("extended instructions", () => {
    it("errors on non-glsl extensions", () => {
      let input = "%1 = OpExtInstImport \"OpenCL.std.100\"";

      let l = new Lexer(input);
      let p = new Parser(grammar, l);

      assert.isUndefined(p.parse());
    });

    it("handles extended instructions", () => {
      let input = `%1 = OpExtInstImport "GLSL.std.450"
  %44 = OpExtInst %7 %1 Sqrt %43`;

      let l = new Lexer(input);
      let p = new Parser(grammar, l);

      let ast = p.parse();
      assert.exists(ast, p.error);
      assert.lengthOf(ast.instructions(), 2);

      let inst = ast.instruction(1);
      assert.lengthOf(inst.operands(), 5);
      assert.equal(inst.operand(3).value(), 31);
      assert.equal(inst.operand(3).name(), "Sqrt");
      assert.equal(inst.operand(4).value(), 43);
      assert.equal(inst.operand(4).name(), "43");
    });
  });

  it.skip("handles spec constant ops", () => {
    // let input = "%sum = OpSpecConstantOp %i32 IAdd %a %b";
  });

  it("handles OpCopyMemory", () => {
    let input = "OpCopyMemory %1 %2 " +
                "Volatile|Nontemporal|MakePointerVisible %3 " +
                "Aligned|MakePointerAvailable|NonPrivatePointer 16 %4";

    let l = new Lexer(input);
    let p = new Parser(grammar, l);

    let ast = p.parse();
    assert.exists(ast, p.error);
    assert.lengthOf(ast.instructions(), 1);

    let inst = ast.instruction(0);
    assert.lengthOf(inst.operands(), 4);
    assert.equal(inst.operand(0).value(), 1);
    assert.equal(inst.operand(1).value(), 2);

    assert.equal(inst.operand(2).name(),
        "Volatile|Nontemporal|MakePointerVisible");
    assert.equal(inst.operand(2).value(), 21);
    assert.lengthOf(inst.operand(2).params(), 1);
    assert.equal(inst.operand(2).params()[0].value(), 3);

    assert.equal(inst.operand(3).name(),
        "Aligned|MakePointerAvailable|NonPrivatePointer");
    assert.equal(inst.operand(3).value(), 42);
    assert.lengthOf(inst.operand(3).params(), 2);
    assert.equal(inst.operand(3).params()[0].value(), 16);
    assert.equal(inst.operand(3).params()[1].value(), 4);
  });
});
