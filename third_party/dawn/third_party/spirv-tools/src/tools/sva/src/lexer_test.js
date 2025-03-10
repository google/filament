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
import { TokenType } from "./token";

describe("lexer", () => {
  describe("skipped content", () => {
    it("skips whitespace", () => {
      let input = " \t\r\n\t  \tOpKill\t\n\t  \r  ";
      let l = new Lexer(input);

      let t = l.next();
      assert.equal(t.type, TokenType.kOp);
      assert.equal(t.line, 2);
      assert.equal(t.data.name, "OpKill");

      t = l.next();
      assert.equal(t.type, TokenType.kEOF);
      assert.equal(t.line, 3);
    });

    it("skips ; comments", () => {
      let input = `; start with comment
OpKill ; end of line comment
; another comment
%1`;

      let l = new Lexer(input);
      let t = l.next();
      assert.equal(t.type, TokenType.kOp);
      assert.equal(t.data.name, "OpKill");
      assert.equal(t.line, 2);

      t = l.next();
      assert.equal(t.type, TokenType.kResultId);
      assert.equal(t.data.name, "1");
      assert.equal(t.data.val, 1);
      assert.equal(t.line, 4);
    });
  });

  describe("numerics", () => {
    it("parses floats", () => {
      let input = ["0.0", "0.", ".0", "5.7", "5.", ".7", "-0.0", "-.0",
        "-0.", "-5.7", "-5.", "-.7"];

      let results = [0.0, 0.0, 0.0, 5.7, 5.0, 0.7, 0.0, 0.0, 0.0, -5.7, -5.0,
        -0.7];
      input.forEach((val, idx) => {
        let l = new Lexer(val);
        let t = l.next();

        assert.equal(t.type, TokenType.kFloatLiteral,
          `expected ${val} to be a float got ${t.type}`);
        assert.equal(t.data, results[idx],
          `expected ${results[idx]} === ${t.data}`);

        t = l.next();
        assert.equal(t.type, TokenType.kEOF);
        assert.equal(t.data, undefined);
      });
    });

    it("handles invalid floats", () => {
      let input = [".", "-."];
      input.forEach((val) => {
        let l = new Lexer(val);
        let t = l.next();

        assert.notEqual(t.type, TokenType.kFloatLiteral,
          `expect ${val} to not match type float`);
      });
    });

    it("parses integers", () => {
      let input = ["0", "-0", "123", "-123", "2147483647", "-2147483648",
        "4294967295", "0x00", "0x24"];
      let results = [0, 0, 123, -123,2147483647, -2147483648, 4294967295,
        0x0, 0x24];

      input.forEach((val, idx) => {
        let l = new Lexer(val);
        let t = l.next();

        assert.equal(t.type, TokenType.kIntegerLiteral,
          `expected ${val} to be an integer got ${t.type}`);
        assert.equal(t.data, results[idx],
          `expected ${results[idx]} === ${t.data}`);

        t = l.next();
        assert.equal(t.type, TokenType.kEOF);
        assert.equal(t.data, undefined);
      });
    });
  });

  it("matches result_ids", () => {
    let input = `%123
%001
%main
%_a_b_c`;

    let result = [
      {name: "123", val: 123},
      {name: "001", val: 1},
      {name: "main", val: undefined},
      {name: "_a_b_c", val: undefined}
    ];

    let l = new Lexer(input);
    for (let i = 0; i < result.length; ++i) {
      let t = l.next();
      assert.equal(t.type, TokenType.kResultId);
      assert.equal(t.data.name, result[i].name);
      assert.equal(t.data.val, result[i].val);
    }
  });

  it("matches punctuation", () => {
    let input = "=";
    let results = [TokenType.kEqual];

    let l = new Lexer(input);
    for (let i = 0; i < results.length; ++i) {
      let t = l.next();
      assert.equal(t.type, results[i]);
      assert.equal(t.line, i + 1);
    }

    let t = l.next();
    assert.equal(t.type, TokenType.kEOF);
  });

  describe("strings", () => {
    it("matches strings", () => {
      let input = "\"GLSL.std.450\"";

      let l = new Lexer(input);
      let t = l.next();
      assert.equal(t.type, TokenType.kStringLiteral);
      assert.equal(t.data, "GLSL.std.450");
    });

    it("handles unfinished strings", () => {
      let input = "\"GLSL.std.450";

      let l = new Lexer(input);
      let t = l.next();
      assert.equal(t.type, TokenType.kError);
    });

    it("handles escapes", () => {
      let input = `"embedded\\"quote"
"embedded\\\\slash"
"embedded\\nchar"`;
      let results = [`embedded\"quote`, `embedded\\slash`, `embeddednchar`];

      let l = new Lexer(input);
      for (let i = 0; i < results.length; ++i) {
        let t = l.next();
        assert.equal(t.type, TokenType.kStringLiteral, results[i]);
        assert.equal(t.data, results[i]);
      }
    });
  });

  it("matches keywords", () => {
    let input = "GLSL Function";
    let results = ["GLSL", "Function"];

    let l = new Lexer(input);
    for (let i = 0; i < results.length; ++i) {
      let t = l.next();
      assert.equal(t.type, TokenType.kIdentifier, results[i]);
      assert.equal(t.data, results[i]);
    }
  });
});
