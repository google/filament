// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview
 * LogGroupEntry is a wrapper around log entries, which makes it easier to
 * find the corresponding start/end of events.
 *
 * This is used internally by the log and timeline views to pretty print
 * collections of log entries.
 */

// TODO(eroman): document these methods!

var LogGroupEntry = (function() {
  'use strict';

  function LogGroupEntry(origEntry, index) {
    this.orig = origEntry;
    this.index = index;
  }

  LogGroupEntry.prototype = {
    isBegin: function() {
      return this.orig.phase == EventPhase.PHASE_BEGIN;
    },

    isEnd: function() {
      return this.orig.phase == EventPhase.PHASE_END;
    },

    getDepth: function() {
      var depth = 0;
      var p = this.parentEntry;
      while (p) {
        depth += 1;
        p = p.parentEntry;
      }
      return depth;
    }
  };

  function findParentIndex(parentStack, eventType) {
    for (var i = parentStack.length - 1; i >= 0; --i) {
      if (parentStack[i].orig.type == eventType)
        return i;
    }
    return -1;
  }

  /**
   * Returns a list of LogGroupEntrys. This basically wraps the original log
   * entry, but makes it easier to find the start/end of the event.
   */
  LogGroupEntry.createArrayFrom = function(origEntries) {
    var groupedEntries = [];

    // Stack of enclosing PHASE_BEGIN elements.
    var parentStack = [];

    for (var i = 0; i < origEntries.length; ++i) {
      var origEntry = origEntries[i];

      var groupEntry = new LogGroupEntry(origEntry, i);
      groupedEntries.push(groupEntry);

      // If this is the end of an event, match it to the start.
      if (groupEntry.isEnd()) {
        // Walk up the parent stack to find the corresponding BEGIN for this
        // END.
        var parentIndex = findParentIndex(parentStack, groupEntry.orig.type);

        if (parentIndex == -1) {
          // Unmatched end.
        } else {
          groupEntry.begin = parentStack[parentIndex];

          // Consider this as the terminator for all open BEGINs up until
          // parentIndex.
          while (parentIndex < parentStack.length) {
            var p = parentStack.pop();
            p.end = groupEntry;
          }
        }
      }

      // Inherit the current parent.
      if (parentStack.length > 0)
        groupEntry.parentEntry = parentStack[parentStack.length - 1];

      if (groupEntry.isBegin())
        parentStack.push(groupEntry);
    }

    return groupedEntries;
  };

  return LogGroupEntry;
})();

