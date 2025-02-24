// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Class to handle display and placement of a div that appears under the cursor
 * when it overs over a specied element.  The div always appears below and to
 * the left of the cursor.
 */
var MouseOverHelp = (function() {
  'use strict';

  /**
   * @param {string} helpDivId Name of the div to position and display
   * @param {string} mouseOverElementId Name the element that displays the
   *     |helpDivId| div on mouse over.
   * @constructor
   */
  function MouseOverHelp(helpDivId, mouseOverElementId) {
    this.node_ = $(helpDivId);

    $(mouseOverElementId).onmouseover = this.onMouseOver.bind(this);
    $(mouseOverElementId).onmouseout = this.onMouseOut.bind(this);

    this.show(false);
  }

  MouseOverHelp.prototype = {
    /**
     * Positions and displays the div, if not already visible.
     * @param {MouseEvent} event Mouse event that triggered the call.
     */
    onMouseOver: function(event) {
      if (this.isVisible_)
        return;

      this.node_.style.position = 'absolute';

      this.show(true);

      this.node_.style.left = (event.clientX + 15).toFixed(0) + 'px';
      this.node_.style.top = event.clientY.toFixed(0) + 'px';
    },

    /**
     * Hides the div when the cursor leaves the hover element.
     * @param {MouseEvent} event Mouse event that triggered the call.
     */
    onMouseOut: function(event) {
      this.show(false);
    },

    /**
     * Sets the div's visibility.
     * @param {boolean} isVisible True if the help div should be shown.
     */
    show: function(isVisible) {
      setNodeDisplay(this.node_, isVisible);
      this.isVisible_ = isVisible;
    },
  };

  return MouseOverHelp;
})();

