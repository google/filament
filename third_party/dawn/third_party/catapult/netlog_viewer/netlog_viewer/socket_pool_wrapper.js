// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var SocketPoolWrapper = (function() {
  'use strict';

  /**
   * SocketPoolWrapper is a wrapper around socket pools entries.  It's
   * used by the log and sockets view to print tables containing both
   * a synopsis of the state of all pools, and listing the groups within
   * individual pools.
   *
   * The constructor takes a socket pool and its parent, and generates a
   * unique name from the two, which is stored as |fullName|.  |parent|
   * must itself be a SocketPoolWrapper.
   *
   * @constructor
   */
  function SocketPoolWrapper(socketPool, parent) {
    this.origPool = socketPool;
    this.fullName = socketPool.name;
    if (this.fullName != socketPool.type)
      this.fullName += ' (' + socketPool.type + ')';
    if (parent)
      this.fullName = parent.fullName + '->' + this.fullName;
  }

  /**
   * Returns an array of SocketPoolWrappers created from each of the socket
   * pools in |socketPoolInfo|.  Nested socket pools appear immediately after
   * their parent, and groups of nodes from trees with root nodes with the same
   * id are placed adjacent to each other.
   */
  SocketPoolWrapper.createArrayFrom = function(socketPoolInfo) {
    // Create SocketPoolWrappers for each socket pool and separate socket pools
    // them into different arrays based on root node name.
    var socketPoolGroups = [];
    var socketPoolNameLists = {};
    for (var i = 0; i < socketPoolInfo.length; ++i) {
      var name = socketPoolInfo[i].name;
      if (!socketPoolNameLists[name]) {
        socketPoolNameLists[name] = [];
        socketPoolGroups.push(socketPoolNameLists[name]);
      }
      addSocketPoolsToList(socketPoolNameLists[name], socketPoolInfo[i], null);
    }

    // Merge the arrays.
    var socketPoolList = [];
    for (var i = 0; i < socketPoolGroups.length; ++i) {
      socketPoolList = socketPoolList.concat(socketPoolGroups[i]);
    }
    return socketPoolList;
  };

  /**
   * Recursively creates SocketPoolWrappers from |origPool| and all its
   * children and adds them all to |socketPoolList|.  |parent| is the
   * SocketPoolWrapper for the parent of |origPool|, or null, if it's
   * a top level socket pool.
   */
  function addSocketPoolsToList(socketPoolList, origPool, parent) {
    var socketPool = new SocketPoolWrapper(origPool, parent);
    socketPoolList.push(socketPool);
    if (origPool.nested_pools) {
      for (var i = 0; i < origPool.nested_pools.length; ++i) {
        addSocketPoolsToList(
            socketPoolList, origPool.nested_pools[i], socketPool);
      }
    }
  }

  /**
   * Returns a table printer containing information on each
   * SocketPoolWrapper in |socketPools|.
   */
  SocketPoolWrapper.createTablePrinter = function(socketPools) {
    var tablePrinter = new TablePrinter();
    tablePrinter.addHeaderCell('Name');
    tablePrinter.addHeaderCell('Handed Out');
    tablePrinter.addHeaderCell('Idle');
    tablePrinter.addHeaderCell('Connecting');
    tablePrinter.addHeaderCell('Max');
    tablePrinter.addHeaderCell('Max Per Group');
    tablePrinter.addHeaderCell('Generation');

    for (var i = 0; i < socketPools.length; i++) {
      var origPool = socketPools[i].origPool;

      tablePrinter.addRow();
      tablePrinter.addCell(socketPools[i].fullName);

      tablePrinter.addCell(origPool.handed_out_socket_count);
      var idleCell = tablePrinter.addCell(origPool.idle_socket_count);
      var connectingCell =
          tablePrinter.addCell(origPool.connecting_socket_count);

      if (origPool.groups) {
        var idleSources = [];
        var connectingSources = [];
        for (var groupName in origPool.groups) {
          var group = origPool.groups[groupName];
          idleSources = idleSources.concat(group.idle_sockets);
          connectingSources = connectingSources.concat(group.connect_jobs);
        }
        idleCell.link = sourceListLink(idleSources);
        connectingCell.link = sourceListLink(connectingSources);
      }

      tablePrinter.addCell(origPool.max_socket_count);
      tablePrinter.addCell(origPool.max_sockets_per_group);
      tablePrinter.addCell(origPool.pool_generation_number);
    }
    return tablePrinter;
  };

  SocketPoolWrapper.prototype = {
    /**
     * Returns a table printer containing information on all a
     * socket pool's groups.
     */
    createGroupTablePrinter: function() {
      var tablePrinter = new TablePrinter();
      tablePrinter.setTitle(this.fullName);

      tablePrinter.addHeaderCell('Name');
      tablePrinter.addHeaderCell('Pending');
      tablePrinter.addHeaderCell('Top Priority');
      tablePrinter.addHeaderCell('Active');
      tablePrinter.addHeaderCell('Idle');
      tablePrinter.addHeaderCell('Connect Jobs');
      tablePrinter.addHeaderCell('Backup Timer');
      tablePrinter.addHeaderCell('Stalled');

      for (var groupName in this.origPool.groups) {
        var group = this.origPool.groups[groupName];

        tablePrinter.addRow();
        tablePrinter.addCell(groupName);
        tablePrinter.addCell(group.pending_request_count);
        if (group.top_pending_priority != undefined)
          tablePrinter.addCell(group.top_pending_priority);
        else
          tablePrinter.addCell('-');

        tablePrinter.addCell(group.active_socket_count);
        var idleCell = tablePrinter.addCell(group.idle_sockets.length);
        var connectingCell = tablePrinter.addCell(group.connect_jobs.length);

        idleCell.link = sourceListLink(group.idle_sockets);
        connectingCell.link = sourceListLink(group.connect_jobs);

        tablePrinter.addCell(
            group.backup_job_timer_is_running ? 'started' : 'stopped');
        tablePrinter.addCell(group.is_stalled);
      }
      return tablePrinter;
    }
  };

  /**
   * Takes in a list of source IDs and returns a link that will select the
   * specified sources.
   */
  function sourceListLink(sources) {
    if (!sources.length)
      return null;
    return '#events&q=id:' + sources.join(',');
  }

  return SocketPoolWrapper;
})();

