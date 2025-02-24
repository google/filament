// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * TODO(eroman): This is an old abstraction, switch to custom element instead.
 */
var View = (function() {
  'use strict';

  /**
   * @constructor
   */
  function View() {
    this.isVisible_ = true;
  }

  View.prototype = {
    /**
     * Called to show/hide the view.
     */
    show: function(isVisible) {
      this.isVisible_ = isVisible;
    },

    isVisible: function() {
      return this.isVisible_;
    },

    /**
     * Method of the observer class.
     *
     * Called to check if an observer needs the data it is
     * observing to be actively updated.
     */
    isActive: function() {
      return this.isVisible();
    },

    setParameters: function(params) {},

    /**
     * Called when loading a log file, after clearing all events, but before
     * loading the new ones.  |polledData| contains the data from all
     * PollableData helpers.  |tabData| contains the data for the particular
     * tab.  |logDump| is the entire log dump, which includes the other two
     * values.  It's included separately so most views don't have to depend on
     * its specifics.
     */
    onLoadLogStart: function(polledData, tabData, logDump) {},

    /**
     * Called as the final step of loading a log file.  Arguments are the same
     * as onLoadLogStart.  Returns true to indicate the tab should be shown,
     * false otherwise.
     */
    onLoadLogFinish: function(polledData, tabData, logDump) {
      return false;
    }
  };

  return View;
})();

//-----------------------------------------------------------------------------

/**
 * TODO(eroman): This is an old abstraction, switch to custom element instead.
 */
var DivView = (function() {
  'use strict';

  // We inherit from View.
  var superClass = View;

  /**
   * @constructor
   */
  function DivView(divId) {
    // Call superclass's constructor.
    superClass.call(this);

    this.node_ = $(divId);
    if (!this.node_)
      throw new Error('Element ' + divId + ' not found');

    this.isVisible_ = this.node_.style.display != 'none';
  }

  DivView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    show: function(isVisible) {
      superClass.prototype.show.call(this, isVisible);

      // TODO(eroman): In the process of getting rid of this view
      // hiearchy and just using custom elements directly. DivView is mostly
      // used to represent a custom element, however the root node is the first
      // node within the custom element, rather than the custom element itself.
      // To work around this, if it looks like this is the root of a custom
      // element, change the visibility of the custom element.
      let n = this.node_;
      if (this.node_.parentNode.nodeName.includes('-')) {
        n = this.node_.parentNode;
      }

      setNodeDisplay(n, isVisible);
    },

    /**
     * Returns the wrapped DIV
     */
    getNode: function() {
      return this.node_;
    }
  };

  return DivView;
})();

