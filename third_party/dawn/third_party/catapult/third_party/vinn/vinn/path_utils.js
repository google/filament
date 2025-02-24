// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

/**
 * @fileoverview Provides tools for working with paths, needed for d8
 * bootstrapping process.
 *
 * This file is pure js instead of an HTML imports module,
 * and writes to the global namespace so that it can
 * be included directly by the boostrap.
 */
(function(global) {
  /**
   *  Class provides common operations on pathnames.
   *  @param {object} os_client An object that defines:
   *    currentWorkingDirectory: the current working directory of program.
   *    exists: a function that takes a string fileName and returns whether the
   *    file with name exists.
   *
   *  @constructor
   */
  global.PathUtils = function(os_client) {
    this.os_client_ = os_client;
    var path = this.os_client_.currentWorkingDirectory;
    if (!this.isAbs(path)) {
      throw new Error('Invalid current directory: ' + path + '. ' +
          'It must be an absolute path.');
    }
  }

  global.PathUtils.prototype = {
    __proto__: Object.prototype,

    get currentWorkingDirectory() {
      return this.os_client_.currentWorkingDirectory;
    },

    exists: function(fileName) {
      return this.os_client_.exists(this.absPath(fileName));
    },

    isAbs: function(a) {
      return a[0] === '/' || a[1] === ':';
    },

    join: function(a, b) {
      if (this.isAbs(b))
        return b;

      var res = a;
      if (!a.endsWith('/'))
        res += '/';

      res += b;
      return res;
    },

    normPath: function(a) {
      if (a.endsWith('/'))
        return a.substring(0, a.length - 1);
      return a;
    },

    /* TODO(crbug.com/1111556): fix this implementation on windows */
    absPath: function(a) {
      if (this.isAbs(a))
        return a;

      if (a.startsWith('./'))
        a = a.substring(2);
      var res = this.join(this.currentWorkingDirectory, a);
      return this.normPath(res);
    },

    relPath: function(a, opt_relTo) {
      var a = this.absPath(a);

      var relTo;
      if (opt_relTo) {
        relTo = this.normPath(this.absPath(opt_relTo));
      } else {
        relTo = this.currentWorkingDirectory;
      }

      if (relTo.endsWith('/'))
        relTo = relTo.substring(0, relTo.length - 1);

      if (!a.startsWith(relTo)) {
        var parts = relTo.substring(1).split('/');
        for (var i = 0; i < parts.length; i++)
          parts[i] = '..';
        var prefix = parts.join('/');

        var suffix;
        if (a.endsWith('/'))
          suffix = a.substring(0, a.length - 1);
        else
          suffix = a;
        return prefix + suffix;
      }

      var rest = a.substring(relTo.length + 1);
      if (rest.length === 0)
        return '.';
      return this.normPath(rest);
    }
  };
})(this);
