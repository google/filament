// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays information related to Prerendering.
 */
var PrerenderView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function PrerenderView() {
    assertFirstConstructorCall(PrerenderView);

    // Call superclass's constructor.
    superClass.call(this, PrerenderView.MAIN_BOX_ID);

    g_browser.addPrerenderInfoObserver(this, true);
  }

  PrerenderView.TAB_ID = 'tab-handle-prerender';
  PrerenderView.TAB_NAME = 'Prerender';
  PrerenderView.TAB_HASH = '#prerender';

  // IDs for special HTML elements in prerender_view.html
  PrerenderView.MAIN_BOX_ID = 'prerender-view-tab-content';
  PrerenderView.PRERENDER_VIEW_ENABLED_ID =
      'prerender-view-enabled';
  PrerenderView.PRERENDER_VIEW_ENABLED_NOTE_ID =
      'prerender-view-enabled-note';
  PrerenderView.PRERENDER_VIEW_OMNIBOX_ENABLED_ID =
      'prerender-view-omnibox-enabled';
  PrerenderView.HISTORY_TABLE_ID = 'prerender-view-history-table';
  PrerenderView.ACTIVE_TABLE_ID = 'prerender-view-active-table';

  cr.addSingletonGetter(PrerenderView);

  PrerenderView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onLoadLogFinish: function(data) {
      return this.onPrerenderInfoChanged(data.prerenderInfo);
    },

    onPrerenderInfoChanged: function(prerenderInfo) {
      if (!prerenderInfo)
        return false;

      $(PrerenderView.PRERENDER_VIEW_ENABLED_ID).textContent =
          prerenderInfo.enabled;
      $(PrerenderView.PRERENDER_VIEW_ENABLED_NOTE_ID).textContent =
          prerenderInfo.enabled_note;
      $(PrerenderView.PRERENDER_VIEW_OMNIBOX_ENABLED_ID).textContent =
          prerenderInfo.omnibox_enabled;

      var tbodyActive = $(PrerenderView.ACTIVE_TABLE_ID);
      tbodyActive.innerHTML = '';

      // Fill in Active Prerender Pages table
      for (var i = 0; i < prerenderInfo.active.length; ++i) {
        var a = prerenderInfo.active[i];
        var tr = addNode(tbodyActive, 'tr');

        addNodeWithText(tr, 'td', a.url);
        addNodeWithText(tr, 'td', a.duration);
        addNodeWithText(tr, 'td', a.is_loaded);
      }

      var tbodyHistory = $(PrerenderView.HISTORY_TABLE_ID);
      tbodyHistory.innerHTML = '';

      // Fill in Prerender History table
      for (var i = 0; i < prerenderInfo.history.length; ++i) {
        var h = prerenderInfo.history[i];
        var tr = addNode(tbodyHistory, 'tr');
        tr.className = h.final_status.toLowerCase();

        addNodeWithText(tr, 'td', h.origin);
        addNodeWithText(tr, 'td', h.url);
        addNodeWithText(tr, 'td', h.final_status);
        addNodeWithText(tr, 'td',
            timeutil.dateToString(new Date(parseInt(h.end_time))));
      }

      return true;
    }
  };

  return PrerenderView;
})();
