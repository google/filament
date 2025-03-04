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

import { TokenType } from "./token.js";
import * as AST from "./ast.js";

export default class Parser {
  /**
   * @param {Hash} The SPIR-V grammar
   * @param {Lexer} The lexer
   * @return {AST} Attempts to build an AST from the tokens returned by the
   *               given lexer
   */
  constructor(grammar, lexer) {
    this.grammar_ = grammar;
    this.lexer_ = lexer;

    this.peek_ = [];
    this.error_ = "";
  }

  get error() { return this.error_; }

  next() {
    return this.peek_.shift() || this.lexer_.next();
  }

  peek(idx) {
    while (this.peek_.length <= idx) {
      this.peek_.push(this.lexer_.next());
    }
    return this.peek_[idx];
  }

  /**
   * Executes the parser.
   *
   * @return {AST|undefined} returns a parsed AST on success or undefined
   *                         on error. The error message can be retrieved by
   *                         calling error().
   */
  parse() {
    let ast = new AST.Module();
    for(;;) {
      let token = this.next();
      if (token === TokenType.kError) {
        this.error_ = token.line() + ": " + token.data();
        return undefined;
      }
      if (token.type === TokenType.kEOF)
        break;

      let result_id = undefined;
      if (token.type === TokenType.kResultId) {
        result_id = token;

        token = this.next();
        if (token.type !== TokenType.kEqual) {
          this.error_ = token.line + ": expected = after result id";
          return undefined;
        }

        token = this.next();
      }

      if (token.type !== TokenType.kOp) {
        this.error_ = token.line + ": expected Op got " + token.type;
        return undefined;
      }

      let name = token.data.name;
      let data = this.getInstructionData(name);
      let operands = [];
      let result_type = undefined;

      for (let operand of data.operands) {
        if (operand.kind === "IdResult") {
          if (result_id === undefined) {
            this.error_ = token.line + ": expected result id";
            return undefined;
          }
          let o = new AST.Operand(ast, result_id.data.name, "result_id",
            result_id.data.val, []);
          if (o === undefined) {
            return undefined;
          }
          operands.push(o);
        } else {
          if (operand.quantifier === "?") {
            if (this.nextIsNewInstr()) {
              break;
            }
          } else if (operand.quantifier === "*") {
            while (!this.nextIsNewInstr()) {
              let o = this.extractOperand(ast, result_type, operand);
              if (o === undefined) {
                return undefined;
              }
              operands.push(o);
            }
            break;
          }

          let o = this.extractOperand(ast, result_type, operand);
          if (o === undefined) {
            return undefined;
          }

          // Store the result type away so we can use it for context dependent
          // numbers if needed.
          if (operand.kind === "IdResultType") {
            result_type = ast.getType(o.name());
          }

          operands.push(o);
        }
      }

      // Verify only GLSL extended instructions are used
      if (name === "OpExtInstImport" && operands[1].value() !== "GLSL.std.450") {
        this.error_ = token.line + ": Only GLSL.std.450 external instructions supported";
        return undefined;
      }

      let inst = new AST.Instruction(name, data.opcode, operands);

      ast.addInstruction(inst);
    }
    return ast;
  }

  getInstructionData(name) {
    return this.grammar_["instructions"][name];
  }

  nextIsNewInstr() {
    let n0 = this.peek(0);
    if (n0.type === TokenType.kOp || n0.type === TokenType.kEOF) {
      return true;
    }

    let n1 = this.peek(1);
    if (n1.type === TokenType.kEOF) {
      return false;
    }
    if (n0.type === TokenType.kResultId && n1.type === TokenType.kEqual)
      return true;

    return false;
  }

  extractOperand(ast, result_type, data) {
    let t = this.next();

    let name = undefined;
    let kind = undefined;
    let value = undefined;
    let params = [];

    // TODO(dsinclair): There are a bunch of missing types here. See
    // https://github.com/KhronosGroup/SPIRV-Tools/blob/master/source/text.cpp#L210
    //
    // LiteralSpecConstantOpInteger
    // PairLiteralIntegerIdRef
    // PairIdRefLiteralInteger
    // PairIdRefIdRef
    if (data.kind === "IdResult" || data.kind === "IdRef"
        || data.kind === "IdResultType" || data.kind === "IdScope"
        || data.kind === "IdMemorySemantics") {
      if (t.type !== TokenType.kResultId) {
        this.error_ = t.line + ": expected result id";
        return undefined;
      }

      name = t.data.name;
      kind = "result_id";
      value = t.data.val;
    } else if (data.kind === "LiteralString") {
      if (t.type !== TokenType.kStringLiteral) {
        this.error_ = t.line + ": expected string not found";
        return undefined;
      }

      name = t.data;
      kind = "string";
      value = t.data;
    } else if (data.kind === "LiteralInteger") {
      if (t.type !== TokenType.kIntegerLiteral) {
        this.error_ = t.line + ": expected integer not found";
        return undefined;
      }

      name = "" + t.data;
      kind = t.type;
      value = t.data;
    } else if (data.kind === "LiteralContextDependentNumber") {
      if (result_type === undefined) {
        this.error_ = t.line +
            ": missing result type for context dependent number";
        return undefined;
      }
      if (t.type !== TokenType.kIntegerLiteral
          && t.type !== TokenType.kFloatLiteral) {
        this.error_ = t.line + ": expected number not found";
        return undefined;
      }

      name = "" + t.data;
      kind = result_type.type;
      value = t.data;

    } else if (data.kind === "LiteralExtInstInteger") {
      if (t.type !== TokenType.kIdentifier) {
        this.error_ = t.line + ": expected instruction identifier";
        return undefined;
      }

      if (this.grammar_.ext[t.data] === undefined) {
        this.error_ = t.line + `: unable to find extended instruction (${t.data})`;
        return undefined;
      }

      name = t.data;
      kind = "integer";
      value = this.grammar_.ext[t.data];

    } else {
      let d = this.grammar_.operand_kinds[data.kind];
      if (d === undefined) {
        this.error_ = t.line + ": expected " + data.kind + " not found";
        return undefined;
      }

      let val = d.values[t.data]["value"];
      let names = [t.data];
      if (d.type === "BitEnum") {
        for(;;) {
          let tmp = this.peek(0);
          if (tmp.type !== TokenType.kPipe) {
            break;
          }

          this.next();  // skip pipe
          tmp = this.next();

          if (tmp.type !== TokenType.kIdentifier) {
            this.error_ = tmp.line() + ": expected identifier";
            return undefined;
          }

          val |= d.values[tmp.data]["value"];
          names.push(tmp.data);
        }
      }

      name = names.join("|");
      kind = d.type;
      value = val;

      for (const op_name of names) {
        if (d.values[op_name]['params'] === undefined) {
          continue;
        }

        for (const param of d.values[op_name]["params"]) {
          params.push(this.extractOperand(ast, result_type, { kind: param }));
        }
      }
    }
    return new AST.Operand(ast, name, kind, value, params);
  }
}
