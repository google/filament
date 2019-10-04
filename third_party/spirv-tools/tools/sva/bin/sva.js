#!/usr/bin/env node
//
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

"use strict";

const fs = require("fs");

import SVA from "../src/sva.js";

let input = fs.readFileSync(process.argv[2], "utf-8");
let u = SVA.assemble(input);

if (typeof u === "string") {
  console.log(u);
} else {
  fs.writeFileSync("o.sva", new Buffer(u.buffer), (err) => {
    console.log(["ERROR", err]);
  });
}
