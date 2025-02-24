// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * @fileoverview Provides tools for loading an HTML import file into D8.
 */
(function(global) {
  var isNode = global.process && global.process.versions.node;

  var fs;
  if (isNode)
    fs = require('fs');

  var pathUtils = undefined;
  function setPathUtils(newPathUtils) {
    if (pathUtils !== undefined)
      throw new Error('Already set');
    pathUtils = newPathUtils;
  }

  function readFileContents(fileName) {
    if (!isNode)
      return read(fileName);
    return fs.readFileSync(fileName, 'utf8');
  }

  /**
   * Strips the starting '/' in file_path if |file_path| is meant to be a
   * relative path.
   *
   * @param {string} file_path path to some file, can be relative or absolute
   * path.
   * @return {string} the file_path with starting '/' removed if |file_path|
   * does not exist or the original |file_path| otherwise.
   */
  function _stripStartingSlashIfNeeded(file_path) {
    if (file_path.substring(0, 1) !== '/') {
      return file_path;
    }
    if (pathUtils.exists(file_path))
      return file_path;
    return file_path.substring(1);
  }

  var sourcePaths = [];

  function addArrayToSourcePath(paths) {
    for (var i = 0; i < paths.length; i++) {
      if (sourcePaths.indexOf(paths[i]) !== -1)
        continue;
      sourcePaths.push(paths[i]);
    }
  }

  function hrefToAbsolutePath(href) {
    var pathPart;
    if (!pathUtils.isAbs(href)) {
      throw new Error('Found a non absolute import and thats not supported: ' +
                      href);
    } else {
      pathPart = href.substring(1);
    }

    var candidates = [];
    for (var i = 0; i < sourcePaths.length; i++) {
      var candidate = pathUtils.join(sourcePaths[i], pathPart);
      if (pathUtils.exists(candidate))
        candidates.push(candidate);
    }
    if (candidates.length > 1) {
      throw new Error('Multiple candidates found for ' + href + ': ' +
          candidates + '\nSource paths:\n' + sourcePaths.join(',\n'));
    }
    if (candidates.length === 0) {
      throw new Error(href + ' not found!' +
          '\nSource paths:\n' + sourcePaths.join(',\n'));
    }
    return candidates[0];
  }

  var loadedModulesByFilePath = {};

  /**
   * Load a HTML file, which absolute path or path relative to <%search-path%>.
   * Unlike the native load() method of d8, variables declared in |file_path|
   * will not be hoisted to the caller environment. For example:
   *
   * a.html:
   * <script>
   *   var x = 1;
   * </script>
   *
   * test.js:
   * loadHTML("a.html");
   * print(x);  // <- ReferenceError is thrown because x is not defined.
   *
   * @param {string} file_path path to the HTML file to be loaded.
   */
  function loadHTML(href) {
    var absPath = hrefToAbsolutePath(href);
    loadHTMLFile.call(global, absPath, href);
  };

  function loadScript(href) {
    var absPath = hrefToAbsolutePath(href);
    loadFile.call(global, absPath, href);
  };

  function loadHTMLFile(absPath, opt_href) {
    var href = opt_href || absPath;
    if (loadedModulesByFilePath[absPath])
      return;
    loadedModulesByFilePath[absPath] = true;
    try {
      var html_content = readFileContents(absPath);
    } catch (err) {
      throw new Error('Error in loading html file ' + href +
          ': File does not exist');
    }

    try {
      var stripped_js = generateJsFromHTML(html_content);
    } catch (err) {
      throw new Error('Error in loading html file ' + href + ': ' + err);
    }

    // If there is blank line at the beginning of generated js, we add
    // "//@ sourceURL=|file_path|" to the beginning of generated source so
    // the stack trace show the source file even in case of syntax error.
    // If not, we add it to the end of generated source to preserve the line
    // number.
    if (stripped_js.startsWith('\n')) {
      stripped_js = '//@ sourceURL=' + href + stripped_js;
    } else {
      stripped_js = stripped_js + '\n//@ sourceURL=' + href;
    }
    eval(stripped_js);
  };

  function loadFile(absPath, opt_href) {
    var href = opt_href || absPath;
    try {
      if (!isNode) {
        load(absPath);
      } else {
        var relPath = pathUtils.relPath(absPath);
        var contents = readFileContents(relPath);
        eval(contents);
      }
    } catch (err) {
      throw new Error('Error in loading script file ' + href + ': ' + err);
    }
  };

  global.HTMLImportsLoader = {
    setPathUtils: setPathUtils,
    sourcePaths: sourcePaths,
    addArrayToSourcePath: addArrayToSourcePath,
    hrefToAbsolutePath: hrefToAbsolutePath,
    loadHTML: loadHTML,
    loadScript: loadScript,
    loadHTMLFile: loadHTMLFile,
    loadFile: loadFile
  };
})(this);
