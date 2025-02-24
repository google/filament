// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * @fileoverview Lets node import catapult HTML-imports-authored modules.
 *
 */
var isNode = global.process && global.process.versions.node;
if (!isNode)
  throw new Error('Only works inside node');

var fs = require('fs');
var path = require('path');
var process = require('process');
var child_process = require('child_process');

var catapultPath = fs.realpathSync(path.join(__dirname, '..'));
var catapultBuildPath = path.join(catapultPath, 'catapult_build');

var vinnPath = path.join(catapultPath, 'third_party', 'vinn');

function loadAndEval(fileName) {
  var contents = fs.readFileSync(fileName, 'utf8');
  (function() {
    eval(contents);
  }).call(global);
}

function initialize() {
  loadAndEval(path.join(vinnPath, 'vinn', 'base64_compat.js'));

  // First, we need to hand-load the HTML imports loader from Vinn,
  // plus a few of its supporting files. These all assume that 'this' is the
  // global object, so eval them with 'this' redirected.
  loadAndEval(path.join(vinnPath, 'third_party', 'parse5', 'parse5.js'));
  loadAndEval(path.join(vinnPath, 'vinn', 'html_to_js_generator.js'));
  loadAndEval(path.join(vinnPath, 'vinn', 'html_imports_loader.js'));
  loadAndEval(path.join(vinnPath, 'vinn', 'path_utils.js'));

  // Now that everything is loaded, we need to set up the loader.
  var pathUtils = new global.PathUtils(
      {
        currentWorkingDirectory: process.cwd(),
        exists: function(fileName) {
          return fs.existsSync(fileName);
        }
      });
  global.HTMLImportsLoader.setPathUtils(pathUtils);
}


/**
 * Gets the source search paths for a catapult project module.
 *
 * @param {String} projectName The project in question.
 * @return {Array} A list of search paths.
 */
module.exports.getSourcePathsForProject = function(projectName) {
  var sourcePathsString = child_process.execFileSync(
      path.join(catapultBuildPath, 'print_project_info'),
      ['--source-paths', projectName]);
  return JSON.parse(sourcePathsString);
};


/**
 * Gets the headless test module filenames for a catapult project module.
 *
 * @param {String} projectName The project in question.
 * @return {Array} A list of module filenames.
 */
module.exports.getHeadlessTestModuleFilenamesForProject =
    function(projectName) {
  var sourcePathsString = child_process.execFileSync(
      path.join(catapultBuildPath, 'print_project_info'),
      ['--headless-test-module-filenames', projectName]);
  return JSON.parse(sourcePathsString);
};

initialize();
