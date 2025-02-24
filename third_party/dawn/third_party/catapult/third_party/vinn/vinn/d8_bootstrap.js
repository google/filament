// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * @fileoverview Boostrap for loading javascript/html files using d8_runner.
 */
(function(global, v8arguments) {
  global.global = global;

  global.isVinn = true;

  // Save the argv in a predictable and stable location.
  global.sys = {
    argv: []
  };
  for (var i = 0; i < v8arguments.length; i++)
    sys.argv.push(v8arguments[i]);

  /* There are four ways a program can finish running in D8:
   * - a) Intentioned exit triggered via quit(0)
   * - b) Intentioned exit triggered via quit(n)
   * - c) Running to end of the script
   * - d) An uncaught exception
   *
   * The exit code of d8 for case a is 0.
   * The exit code of d8 for case b is unsigned(n) & 0xFF
   * The exit code of d8 for case c is 0.
   * The exit code of d8 for case d is 1.
   *
   * D8 runner needs to distinguish between these cases:
   * - a) _ExecuteFileWithD8 should return 0
   * - b) _ExecuteFileWithD8 should return n
   * - c) _ExecuteFileWithD8 should return 0
   * - d) _ExecuteFileWithD8 should raise an Exception
   *
   * The hard one here is d and b with n=1, because they fight for the same
   * return code.
   *
   * Our solution is to monkeypatch quit() s.t. quit(1) becomes exitcode=2.
   * This makes quit(255) disallowed, but it ensures that D8 runner is able
   * to handle the other cases correctly.
   */
  var realQuit = global.quit;
  global.quit = function(exitCode) {
    // Normalize the exit code.
    if (exitCode < 0) {
      exitCode = (exitCode % 256) + 256;
    } else {
      exitCode = exitCode % 256;
    }

    // 255 is reserved due to reasons noted above.
    if (exitCode == 255)
      throw new Error('exitCodes 255 is reserved, sorry.');
    if (exitCode === 0)
      realQuit(0);
    realQuit(exitCode + 1);
  }

  /**
   * Polyfills console's methods.
   */
  var _timeStamps = new Map();
  global.console = {
    log: function() {
      var args = Array.prototype.slice.call(arguments);
      print(args.join(' '));
    },

    info: function() {
      var args = Array.prototype.slice.call(arguments);
      print('Info:', args.join(' '));
    },

    error: function() {
      var args = Array.prototype.slice.call(arguments);
      print('Error:', args.join(' '));
    },

    warn: function() {
      var args = Array.prototype.slice.call(arguments);
      print('Warning:', args.join(' '));
    },

    time: function(timerName) {
      _timeStamps.set(timerName, performance.now());
    },

    timeEnd: function(timerName) {
      var t = _timeStamps.get(timerName);
      _timeStamps.delete(timerName);
      if (!t)
        throw new Error('No such timer name: ' + timerName);
      var duration = performance.now() - t;
      print(timerName + ':', duration + 'ms');
    }
  };

  if (os.chdir) {
    os.chdir = function() {
      throw new Error('Dont do this');
    }
  }

  var path_to_base64_compat = <%base64_compat_path%>;
  load(path_to_base64_compat);

  // We deliberately call eval() on content of parse5.js instead of using load()
  // because load() does not hoist the |global| variable in this method to
  // parse5.js (which export its modules to |global|).
  //
  // This is because d8's load('xyz.js') does not hoist non global varibles in
  // the caller's environment to xyz.js, no matter where load() is called.
  global.path_to_js_parser = <%js_parser_path%>;
  eval(read(global.path_to_js_parser));

  // Bring in html_to_js_generator.
  global.path_to_js_parser = <%js_parser_path%>;
  load(<%html_to_js_generator_js_path%>);

  // Bring in html imports loader.
  load(<%html_imports_loader_js_path%>);
  global.HTMLImportsLoader.addArrayToSourcePath(<%source_paths%>);

  // Bring in path utils.
  load(<%path_utils_js_path%>);
  var pathUtils = new PathUtils(
      {
        currentWorkingDirectory: <%current_working_directory%>,
        exists: function(fileName) {
          try {
            // Try a dummy read to check whether file_path exists.
            readbuffer(fileName);
            return true;
          } catch (err) {
            return false;
          }
        }
      });
  global.HTMLImportsLoader.setPathUtils(pathUtils);

})(this, arguments);
