// Copyright 2015 The Chromium Authors. All rights reserved.
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

function runTests() {

  var test_os_client = {
    currentWorkingDirectory: '/a/b',
    exists: function(fileName) {
      return fileName === '/a/b/file_exists.html';
    }
  };

  var path_utils = new PathUtils(test_os_client);
  assert.equal(path_utils.join('a', 'b'), 'a/b');
  assert.equal(path_utils.join('/a', 'b'), '/a/b');
  assert.equal(path_utils.join('/a/', 'b/'), '/a/b/');
  assert.equal(path_utils.join('/a', '/b/'), '/b/');
  assert.equal(path_utils.join('/a', './b/'), '/a/./b/');
  assert.equal(path_utils.join('/a/', './b/'), '/a/./b/');
  assert.equal(path_utils.join('../', 'b'), '../b');
  assert.equal(path_utils.join('../', 'b/'), '../b/');
  assert.equal(path_utils.join('a', 'b'), 'a/b');

  assert.equal(path_utils.absPath('c'), '/a/b/c');
  assert.equal(path_utils.absPath('./c'), '/a/b/c');
  assert.equal(path_utils.absPath('./c/d'), '/a/b/c/d');

  assert.equal(path_utils.relPath('/a/b/c', '/a/b'), 'c');

  assert.equal(path_utils.relPath('/a/b/c/', '/a/b/c/'), '.');
  assert.equal(path_utils.relPath('/a/b/c', '/a/b/c/'), '.');
  assert.equal(path_utils.relPath('/a/b/c/', '/a/b/c'), '.');
  assert.equal(path_utils.relPath('/a/b/c', '/a/b/c'), '.');

  assert.equal(path_utils.relPath('/a/b/c', '/a'), 'b/c');
  assert.equal(path_utils.relPath('/a/b/c', '/a/'), 'b/c');

  assert.equal(path_utils.relPath('/a/b/c', '/b/c/'), '../../a/b/c');
  assert.equal(path_utils.relPath('/a/b/c', '/b/c'), '../../a/b/c');
  assert.equal(path_utils.relPath('/a/b/c/', '/b/c/'), '../../a/b/c');
  assert.equal(path_utils.relPath('/a/b/c/', '/b/c'), '../../a/b/c');


  assert.equal(path_utils.exists('/a/b/file_exists.html'), true);
  assert.equal(path_utils.exists('file_exists.html'), true);
  assert.equal(path_utils.exists('./file_exists.html'), true);
  assert.equal(path_utils.exists('/a/file_does_not_exists.html'), false);
}
