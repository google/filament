// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Controller and View for switching between tabs.
 *
 * The View part of TabSwitcherView displays the contents of the currently
 * selected tab (only one tab can be active at a time).
 *
 * The controller part of TabSwitcherView hooks up a dropdown menu (i.e. HTML
 * SELECT) to control switching between tabs.
 */
var TabSwitcherView = (function() {
  'use strict';

  // We inherit from View.
  var superClass = View;

  var TAB_LIST_ID = 'tab-list';

  /**
   * @constructor
   *
   * @param {?Function} opt_onTabSwitched Optional callback to run when the
   *                    active tab changes. Called as
   *                    opt_onTabSwitched(oldTabId, newTabId).
   */
  function TabSwitcherView(opt_onTabSwitched) {
    assertFirstConstructorCall(TabSwitcherView);

    this.tabIdToView_ = {};
    this.tabIdToLink_ = {};
    // Map from tab id to the views link visiblity.
    this.tabIdsLinkVisibility_ = new Map();
    this.activeTabId_ = null;

    this.onTabSwitched_ = opt_onTabSwitched;

    // The ideal width of the tab list.  If width is reduced below this, the
    // tab list will be shrunk, but it will be returned to this width once it
    // can be.
    this.tabListWidth_ = $(TAB_LIST_ID).offsetWidth;

    superClass.call(this);
  }

  TabSwitcherView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    show: function(isVisible) {
      superClass.prototype.show.call(this, isVisible);
      var activeView = this.getActiveTabView();
      if (activeView)
        activeView.show(isVisible);
    },

    // ---------------------------------------------

    /**
     * Adds a new tab (initially hidden).  To ensure correct tab list sizing,
     * may only be called before first layout.
     *
     * @param {string} tabId The ID to refer to the tab by.
     * @param {!View} view The tab's actual contents.
     * @param {string} name The name for the menu item that selects the tab.
     */
    addTab: function(tabId, view, name, hash) {
      if (!tabId) {
        throw Error('Must specify a non-false tabId');
      }

      this.tabIdToView_[tabId] = view;
      this.tabIdsLinkVisibility_.set(tabId, true);

      var node = addNodeWithText($(TAB_LIST_ID), 'a', name);
      node.href = hash;
      this.tabIdToLink_[tabId] = node;
      addNode($(TAB_LIST_ID), 'br');

      // Tab content views start off hidden.
      view.show(false);

      this.tabListWidth_ = $(TAB_LIST_ID).offsetWidth;
    },

    showTabLink: function(tabId, isVisible) {
      var wasActive = this.activeTabId_ == tabId;

      setNodeDisplay(this.tabIdToLink_[tabId], isVisible);
      this.tabIdsLinkVisibility_.set(tabId, isVisible);

      if (wasActive && !isVisible) {
        // If the link for active tab is being hidden, then switch to the first
        // tab which is still visible.
        for (var [localTabId, enabled] of this.tabIdsLinkVisibility_) {
          if (enabled) {
            this.switchToTab(localTabId);
            break;
          }
        }
      }
    },

    getAllTabViews: function() {
      return this.tabIdToView_;
    },

    getTabView: function(tabId) {
      return this.tabIdToView_[tabId];
    },

    getActiveTabView: function() {
      return this.tabIdToView_[this.activeTabId_];
    },

    getActiveTabId: function() {
      return this.activeTabId_;
    },

    /**
     * Changes the currently active tab to |tabId|. This has several effects:
     *   (1) Replace the tab contents view with that of the new tab.
     *   (2) Update the dropdown menu's current selection.
     *   (3) Invoke the optional onTabSwitched callback.
     */
    switchToTab: function(tabId) {
      var newView = this.getTabView(tabId);

      if (!newView) {
        throw Error('Invalid tabId');
      }

      var oldTabId = this.activeTabId_;
      this.activeTabId_ = tabId;

      if (oldTabId) {
        this.tabIdToLink_[oldTabId].classList.remove('selected');
        // Hide the previously visible tab contents.
        this.getTabView(oldTabId).show(false);
      }

      this.tabIdToLink_[tabId].classList.add('selected');

      newView.show(this.isVisible());

      if (this.onTabSwitched_)
        this.onTabSwitched_(oldTabId, tabId);
    },
  };

  return TabSwitcherView;
})();

