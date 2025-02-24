// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays information on the state of all socket pools.
 *
 *   - Shows a summary of the state of each socket pool at the top.
 *   - For each pool with allocated sockets or connect jobs, shows all its
 *     groups with any allocated sockets.
 */
var SocketsView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function SocketsView() {
    assertFirstConstructorCall(SocketsView);

    // Call superclass's constructor.
    superClass.call(this, SocketsView.MAIN_BOX_ID);

    g_browser.addSocketPoolInfoObserver(this, true);
    this.socketPoolDiv_ = $(SocketsView.SOCKET_POOL_DIV_ID);
    this.socketPoolGroupsDiv_ = $(SocketsView.SOCKET_POOL_GROUPS_DIV_ID);
  }

  SocketsView.TAB_ID = 'tab-handle-sockets';
  SocketsView.TAB_NAME = 'Sockets';
  SocketsView.TAB_HASH = '#sockets';

  // IDs for special HTML elements in sockets_view.html
  SocketsView.MAIN_BOX_ID = 'sockets-view-tab-content';
  SocketsView.SOCKET_POOL_DIV_ID = 'sockets-view-pool-div';
  SocketsView.SOCKET_POOL_GROUPS_DIV_ID = 'sockets-view-pool-groups-div';

  cr.addSingletonGetter(SocketsView);

  SocketsView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onLoadLogFinish: function(data) {
      return this.onSocketPoolInfoChanged(data.socketPoolInfo);
    },

    onSocketPoolInfoChanged: function(socketPoolInfo) {
      this.socketPoolDiv_.innerHTML = '';
      this.socketPoolGroupsDiv_.innerHTML = '';

      if (!socketPoolInfo)
        return false;

      var socketPools = SocketPoolWrapper.createArrayFrom(socketPoolInfo);
      var tablePrinter = SocketPoolWrapper.createTablePrinter(socketPools);
      tablePrinter.toHTML(this.socketPoolDiv_, 'styled-table');

      // Add table for each socket pool with information on each of its groups.
      for (var i = 0; i < socketPools.length; ++i) {
        if (socketPools[i].origPool.groups != undefined) {
          var p = addNode(this.socketPoolGroupsDiv_, 'p');
          var br = addNode(p, 'br');
          var groupTablePrinter = socketPools[i].createGroupTablePrinter();
          groupTablePrinter.toHTML(p, 'styled-table');
        }
      }
      return true;
    }
  };

  return SocketsView;
})();

