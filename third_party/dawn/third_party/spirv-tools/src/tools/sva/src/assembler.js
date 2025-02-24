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

export default class Assembler {
  static get GENERATOR_ID() { return 0; }

  /**
   * @param {AST} the AST to build the SPIR-V from
   */
  constructor(ast) {
    this.ast_ = ast;
  }

  /**
   * Assembles the AST into binary SPIR-V.
   * @return {Uint32Array} The SPIR-V binary data.
   */
  assemble() {
    let total_size = 5;
    for (const inst of this.ast_.instructions()) {
      total_size += 1;
      for (const op of inst.operands()) {
        total_size += op.length();
      }
    }

    let u = new Uint32Array(total_size);
    u[0] = 0x07230203;  // Magic
    u[1] = 0x00010500;  // Version 1.5
    u[2] = Assembler.GENERATOR_ID;  // Generator magic number
    u[3] = this.ast_.getIdBounds();  // ID bounds
    u[4] = 0;  // Reserved

    let idx = 5;
    for (const inst of this.ast_.instructions()) {
      let op_size = 1;
      for (const op of inst.operands()) {
        op_size += op.length();
      }

      u[idx++] = op_size << 16 | inst.opcode();
      for (const op of inst.operands()) {
        idx = this.processOp(u, idx, op);
      }
    }

    return u;
  }

  processOp(u, idx, op) {
    if (op.type() === "string") {
      let len = 0;
      let v = 0;
      for (const ch of op.value()) {
        v = v | (ch.charCodeAt(0) << (len * 8));
        len += 1;

        if (len === 4) {
          u[idx++] = v;
          len = 0;
          v = 0;
        }
      }
      // Make sure either the terminating 0 byte is written or the last
      // partial word is written.
      u[idx++] = v;

    } else if (op.type() === "float") {
      // TODO(dsinclair): Handle 64 bit floats ...
      let b = new ArrayBuffer(4);
      let f = new Float32Array(b);
      f[0] = op.value();

      let u2 = new Uint32Array(b);

      u[idx++] = u2[0];
    } else {
      u[idx++] = op.value();
    }

    for (const param of op.params()) {
      idx = this.processOp(u, idx, param);
    }

    return idx;
  }
}
