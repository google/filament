// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';
var assert = {
  equal: function(first, second) {
    if (first !== second) {
      throw new Error('Assertion error: ' + JSON.stringify(first) +
                      ' !== ' + JSON.stringify(second));
    }
  }
};

try {
  console.timeEnd("AA");
} catch (err) {
  assert.equal(err.message, "No such timer name: AA");
}

console.time("AA");
console.timeEnd("AA");
try {
  console.timeEnd("AA");
} catch (err) {
  assert.equal(err.message, "No such timer name: AA");
}
