// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays the Alt-Svc mappings.
 */
var AltSvcView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function AltSvcView() {
    assertFirstConstructorCall(AltSvcView);

    // Call superclass's constructor.
    superClass.call(this, AltSvcView.MAIN_BOX_ID);

    g_browser.addAltSvcMappingsObserver(this, true);
  }

  AltSvcView.TAB_ID = 'tab-handle-alt-svc';
  AltSvcView.TAB_NAME = 'Alt-Svc';
  AltSvcView.TAB_HASH = '#alt-svc';

  // IDs for special HTML elements in alt_svc_view.html
  AltSvcView.MAIN_BOX_ID = 'alt-svc-view-tab-content';
  AltSvcView.ALTERNATE_PROTOCOL_MAPPINGS_ID =
      'alt-svc-view-alternate-protocol-mappings';
  AltSvcView.MAPPINGS_CONTENT_ID =
      'alt-svc-view-mappings-content';
  AltSvcView.MAPPINGS_NO_CONTENT_ID =
      'alt-svc-view-mappings-no-content';
  AltSvcView.MAPPINGS_TBODY_ID = 'alt-svc-view-mappings-tbody';

  cr.addSingletonGetter(AltSvcView);

  AltSvcView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onLoadLogFinish: function(data) {
      return this.onAltSvcMappingsChanged(data.altSvcMappings);
    },

    /**
     * Displays the alternate service mappings.
     */
    onAltSvcMappingsChanged: function(altSvcMappings) {
      if (!altSvcMappings)
        return false;

      var hasMappings = altSvcMappings && altSvcMappings.length > 0;

      setNodeDisplay($(AltSvcView.MAPPINGS_CONTENT_ID), hasMappings);
      setNodeDisplay($(AltSvcView.MAPPINGS_NO_CONTENT_ID), !hasMappings);

      var tbody = $(AltSvcView.MAPPINGS_TBODY_ID);
      tbody.innerHTML = '';

      // Fill in the alternate service mappings table.
      for (var i = 0; i < altSvcMappings.length; ++i) {
        var a = altSvcMappings[i];
        var tr = addNode(tbody, 'tr');

        addNodeWithText(tr, 'td', a.server);
        addNodeWithText(tr, 'td', a.alternative_service);
      }

      return true;
    }
  };

  return AltSvcView;
})();
