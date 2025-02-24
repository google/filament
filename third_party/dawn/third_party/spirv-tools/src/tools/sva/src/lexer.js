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

import { Token, TokenType } from "./token.js";

export default class Lexer {
  /**
   * @param {String} input The input string to tokenize.
   */
  constructor(input) {
    this.input_ = input;
    this.len_ = input.length;
    this.cur_pos_ = 0;
    this.cur_line_ = 1;

    this.num_regex_ = /^[0-9]+$/;
    this.alpha_regex_ = /^[a-zA-Z_]+$/;
    this.op_regex_ = /^Op[A-Z][^\s]*$/;
    this.hex_regex_ = /^[0-9a-fA-F]$/;
  }

  /**
   * Parses the next token from the input stream.
   * @return {Token} the next token.
   */
  next() {
    this.skipWhitespace();
    this.skipComments();

    if (this.cur_pos_ >= this.len_)
      return new Token(TokenType.kEOF, this.cur_line_);

    let n = this.tryHexInteger();
    if (n !== undefined)
      return n;

    n = this.tryFloat();
    if (n !== undefined)
      return n;

    n = this.tryInteger();
    if (n !== undefined)
      return n;

    n = this.tryString();
    if (n !== undefined)
      return n;

    n = this.tryOp();
    if (n !== undefined)
      return n;

    n = this.tryPunctuation();
    if (n !== undefined)
      return n;

    n = this.tryResultId();
    if (n !== undefined)
      return n;

    n = this.tryIdent();
    if (n !== undefined)
      return n;

    return new Token(TokenType.kError, this.cur_line_, "Failed to match token");
  }

  is(str) {
    if (this.len_ <= this.cur_pos_ + (str.length - 1))
      return false;

    for (let i = 0; i < str.length; ++i) {
      if (this.input_[this.cur_pos_ + i] !== str[i])
        return false;
    }

    return true;
  }

  isNum(ch) {
    return ch.match(this.num_regex_);
  }

  isAlpha(ch) {
    return ch.match(this.alpha_regex_);
  }

  isAlphaNum(ch) {
    return this.isNum(ch) || this.isAlpha(ch);
  }

  isHex(char) {
    return char.match(this.hex_regex_);
  }

  isCurWhitespace() {
    return this.is(" ") || this.is("\t") || this.is("\r") || this.is("\n");
  }

  skipWhitespace() {
    for(;;) {
      let cur_pos = this.cur_pos_;
      while (this.cur_pos_ < this.len_ &&
          this.isCurWhitespace()) {
        if (this.is("\n"))
          this.cur_line_ += 1;

        this.cur_pos_ += 1;
      }

      this.skipComments();

      // Cursor didn't move so no whitespace matched.
      if (cur_pos === this.cur_pos_)
        break;
    }
  }

  skipComments() {
    if (!this.is(";"))
      return;

    while (this.cur_pos_ < this.len_ && !this.is("\n"))
      this.cur_pos_ += 1;
  }

  /**
   * Attempt to parse the next part of the input as a float.
   * @return {Token|undefined} returns a Token if a float is matched,
   *                           undefined otherwise.
   */
  tryFloat() {
    let start = this.cur_pos_;
    let end = start;

    if (this.cur_pos_ >= this.len_)
      return undefined;
    if (this.input_[end] === "-")
      end += 1;

    while (end < this.len_ && this.isNum(this.input_[end]))
      end += 1;

    // Must have a "." in a float
    if (end >= this.len_ || this.input_[end] !== ".")
      return undefined;

    end += 1;
    while (end < this.len_ && this.isNum(this.input_[end]))
      end += 1;

    let substr = this.input_.substr(start, end - start);
    if (substr === "." || substr === "-.")
      return undefined;

    this.cur_pos_ = end;

    return new Token(TokenType.kFloatLiteral, this.cur_line_, parseFloat(substr));
  }

  /**
   * Attempt to parse a hex encoded integer.
   * @return {Token|undefined} returns a Token if a Hex number is matched,
   *                           undefined otherwise.
   */
  tryHexInteger() {
    let start = this.cur_pos_;
    let end = start;

    if (this.cur_pos_ >= this.len_)
      return undefined;
    if (end + 2 >= this.len_ || this.input_[end] !== "0" ||
        this.input_[end + 1] !== "x") {
      return undefined;
    }

    end += 2;

    while (end < this.len_ && this.isHex(this.input_[end]))
      end += 1;

    this.cur_pos_ = end;

    let val = parseInt(this.input_.substr(start, end - start), 16);
    return new Token(TokenType.kIntegerLiteral, this.cur_line_, val);
  }

  /**
   * Attempt to parse an encoded integer.
   * @return {Token|undefined} returns a Token if a number is matched,
   *                           undefined otherwise.
   */
  tryInteger() {
    let start = this.cur_pos_;
    let end = start;

    if (this.cur_pos_ >= this.len_)
      return undefined;
    if (this.input_[end] === "-")
      end += 1;

    if (end >= this.len_ || !this.isNum(this.input_[end]))
      return undefined;

    while (end < this.len_ && this.isNum(this.input_[end]))
      end += 1;

    this.cur_pos_ = end;

    let val = parseInt(this.input_.substr(start, end - start), 10);
    return new Token(TokenType.kIntegerLiteral, this.cur_line_, val);
  }

  /**
   * Attempt to parse a result id.
   * @return {Token|undefined} returns a Token if a result id is matched,
   *                           undefined otherwise.
   */
  tryResultId() {
    let start = this.cur_pos_;
    if (start >= this.len_)
      return undefined;
    if (!this.is("%"))
      return undefined;

    start += 1;
    this.cur_pos_ += 1;
    while (this.cur_pos_ < this.len_ &&
        (this.isAlphaNum(this.input_[this.cur_pos_]) || this.is("_"))) {
      this.cur_pos_ += 1;
    }

    let ident = this.input_.substr(start, this.cur_pos_ - start);
    let value = undefined;
    if (ident.match(this.num_regex_))
      value = parseInt(ident, 10);

    return new Token(TokenType.kResultId, this.cur_line_, {
      name: ident,
      val: value
    });
  }

  /**
   * Attempt to parse an identifier.
   * @return {Token|undefined} returns a Token if an identifier is matched,
   *                           undefined otherwise.
   */
  tryIdent() {
    let start = this.cur_pos_;
    if (start >= this.len_)
      return undefined;

    while (this.cur_pos_ < this.len_ &&
        (this.isAlphaNum(this.input_[this.cur_pos_]) || this.is("_"))) {
      this.cur_pos_ += 1;
    }

    let ident = this.input_.substr(start, this.cur_pos_ - start);
    return new Token(TokenType.kIdentifier, this.cur_line_, ident);
  }

  /**
   * Attempt to parse an Op command.
   * @return {Token|undefined} returns a Token if an Op command is matched,
   *                           undefined otherwise.
   */
  tryOp() {
    let start = this.cur_pos_;
    if (this.cur_pos_ >= this.len_ || (this.cur_pos_ + 1 >= this.len_))
      return undefined;

    if (this.input_[this.cur_pos_] !== "O" ||
        this.input_[this.cur_pos_ + 1] !== "p") {
      return undefined;
    }

    while (this.cur_pos_ < this.len_ &&
        !this.isCurWhitespace()) {
      this.cur_pos_ += 1;
    }

    return new Token(TokenType.kOp, this.cur_line_, {
      name: this.input_.substr(start, this.cur_pos_ - start)
    });
  }

  /**
   * Attempts to match punctuation strings against the input
   * @return {Token|undefined} Returns the Token for the punctuation or
   *                           undefined if no matches found.
   */
  tryPunctuation() {
    let type = undefined;
    if (this.is("="))
      type = TokenType.kEqual;
    else if (this.is("|"))
      type = TokenType.kPipe;

    if (type === undefined)
      return undefined;

    this.cur_pos_ += type.length;
    return new Token(type, this.cur_line_, type);
  }

  /**
   * Attempts to match strings against the input
   * @return {Token|undefined} Returns the Token for the string or undefined
   *                           if no match found.
   */
  tryString() {
    let start = this.cur_pos_;

    // Must have at least 2 chars for a string.
    if (this.cur_pos_ >= this.len_ || (this.cur_pos_ + 1 >= this.len_))
      return undefined;
    if (!this.is("\""))
      return undefined;

    this.cur_pos_ += 1;
    let str = "";
    while (this.cur_pos_ <= this.len_) {
      if (this.is("\""))
        break;

      if (this.is("\\")) {
        this.cur_pos_ += 1;
        if (this.cur_pos_ >= this.len_)
          return undefined;

        if (this.is("\\")) {
          str += "\\";
        } else if (this.is("\"")) {
          str += '"';
        } else {
          str += this.input_[this.cur_pos_];
        }
      } else {
        str += this.input_[this.cur_pos_];
      }
      this.cur_pos_ += 1;
    }

    if (this.cur_pos_ >= this.len_)
      return undefined;

    this.cur_pos_ += 1;

    return new Token(TokenType.kStringLiteral, this.cur_line_, str);
  }
}
