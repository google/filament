// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * The status view at the top of the page.  It displays what mode net-internals
 * is in (capturing, viewing only, viewing loaded log), and may have extra
 * information and actions depending on the mode.
 */
var TopBarView = (function() {
  'use strict';

  // We inherit from View.
  var superClass = DivView;

  /**
   * Main entry point. Called once the page has loaded.
   * @constructor
   */
  function TopBarView() {
    assertFirstConstructorCall(TopBarView);

    superClass.call(this, TopBarView.BOX_ID);

    this.nameToSubView_ = {
      loaded: new LoadedStatusView()
    };

    this.activeSubView_ = null;
  }

  TopBarView.BOX_ID = 'top-bar-view-content';

  cr.addSingletonGetter(TopBarView);

  TopBarView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    switchToSubView: function(name) {
      var newSubView = this.nameToSubView_[name];

      if (!newSubView)
        throw Error('Invalid subview name');

      var prevSubView = this.activeSubView_;
      this.activeSubView_ = newSubView;

      if (prevSubView)
        prevSubView.show(false);
      newSubView.show(this.isVisible());

      // Let the subview change the color scheme of the top bar.
      $(TopBarView.BOX_ID).className = name + '-status-view';

      return newSubView;
    },
  };

  return TopBarView;
})();

