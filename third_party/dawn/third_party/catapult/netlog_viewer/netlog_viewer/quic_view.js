// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays a summary of the state of each QUIC session, and
 * has links to display them in the events tab.
 */
var QuicView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function QuicView() {
    assertFirstConstructorCall(QuicView);

    // Call superclass's constructor.
    superClass.call(this, QuicView.MAIN_BOX_ID);

    g_browser.addQuicInfoObserver(this, true);
  }

  QuicView.TAB_ID = 'tab-handle-quic';
  QuicView.TAB_NAME = 'QUIC';
  QuicView.TAB_HASH = '#quic';

  // IDs for special HTML elements in quic_view.html
  QuicView.MAIN_BOX_ID = 'quic-view-tab-content';

  QuicView.QUIC_ENABLED_CONTENT_ID =
      'quic-view-quic-enabled-content';
  QuicView.QUIC_ENABLED_NO_CONTENT_ID =
      'quic-view-quic-enabled-no-content';

  QuicView.STATUS_SUPPORTED_VERSIONS = 'quic-view-supported-versions';
  QuicView.STATUS_CONNECTION_OPTIONS = 'quic-view-connection-options';
  QuicView.STATUS_MAX_PACKET_LENGTH = 'quic-view-max-packet-length';
  QuicView.STATUS_IDLE_CONNECTION_TIMEOUTS_SECONDS= 'quic-view-idle-connection-timeout-seconds';
  QuicView.STATUS_REDUCED_PING_TIMEOUTS_SECONDS  = 'quic-view-reduced-ping-timeout-seconds';
  QuicView.STATUS_PACKET_READER_YIELD =
      'quic-view-packet-reader-yield-after-duration-milliseconds';
  QuicView.STATUS_MARK_QUIC_BROKEN =
      'quic-view-mark-quic-broken-when-network-blackholes';
  QuicView.STATUS_DO_NOT_MARK_AS_BROKEN =
      'quic-view-do-not-mark-as-broken-on-network-change';
  QuicView.STATUS_RETRY_WITHOUT_ALT =
      'quic-view-retry-without-alt-svc-on-quic-errors';
  QuicView.STATUS_DO_NOT_FRAGMENT = 'quic-view-do-not-fragment';
  QuicView.STATUS_ALLOW_SERVER_MIGRATION = 'quic-view-allow-server-migration';
  QuicView.STATUS_MIGRATE_SESSIONS_EARLY_V2 =
      'quic-view-migrate-sessions-early-v2';
  QuicView.STATUS_MIGRATE_SESSION_ON_NETWORK_CHANGE_V2 =
      'quic-view-migrate-sessions-on-network-change-v2';
  QuicView.STATUS_RETRANSMITTABLE_ON_WIRE_TIMEOUT =
      'quic-view-retransmittable-on-wire-timeout-milliseconds';
  QuicView.STATUS_DISABLE_BIDIRECTIONAL_STREAMS =
      'quic-view-disable-bidirectional-streams';
  QuicView.STATUS_RACE_CERT_VERIFICATION = 'quic-view-race-cert-verification';
  QuicView.STATUS_RACE_STALE_DNS_ON_CONNECTION = 'quic-view-race-stale-dns-on-connection';
  QuicView.STATUS_ESTIMATE_INITIAL_RTT = 'quic-view-estimate-initial-rtt';
  QuicView.STATUS_FORCE_HOL_BLOCKING = 'quic-view-force-hol-blocking';
  QuicView.STATUS_MAX_SERVER_CONFIGS_STORED_IN_PROPERTIES =
      'quic-view-max-server-configs-stored-in-properties';
  QuicView.STATUS_ORIGINS_TO_FORCE_QUIC_ON =
      'quic-view-origins-to-force-quic-on';
  QuicView.STATUS_SERVER_PUSH_CANCELLATION =
      'quic-view-server-push-cancellation';

  QuicView.SESSION_INFO_CONTENT_ID =
      'quic-view-session-info-content';
  QuicView.SESSION_INFO_NO_CONTENT_ID =
      'quic-view-session-info-no-content';
  QuicView.SESSION_INFO_TBODY_ID = 'quic-view-session-info-tbody';

  cr.addSingletonGetter(QuicView);

  QuicView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onLoadLogFinish: function(data) {
      return this.onQuicInfoChanged(data.quicInfo);
    },

    /**
     * If there are any sessions, display a single table with
     * information on each QUIC session.  Otherwise, displays "None".
     */
    onQuicInfoChanged: function(quicInfo) {
      if (!quicInfo)
        return false;

      setNodeDisplay($(QuicView.QUIC_ENABLED_NO_CONTENT_ID),
          !quicInfo.quic_enabled);
      setNodeDisplay($(QuicView.QUIC_ENABLED_CONTENT_ID),
          !!quicInfo.quic_enabled);

      $(QuicView.STATUS_SUPPORTED_VERSIONS).textContent =
          quicInfo.supported_versions;
      $(QuicView.STATUS_CONNECTION_OPTIONS).textContent =
          quicInfo.connection_options;
      $(QuicView.STATUS_MAX_PACKET_LENGTH).textContent =
          quicInfo.max_packet_length;
      $(QuicView.STATUS_IDLE_CONNECTION_TIMEOUTS_SECONDS).textContent =
          quicInfo.idle_connection_timeout_seconds;
      $(QuicView.STATUS_REDUCED_PING_TIMEOUTS_SECONDS).textContent =
          quicInfo.reduced_ping_timeout_seconds;
      $(QuicView.STATUS_PACKET_READER_YIELD).textContent =
          quicInfo.packet_reader_yield_after_duration_milliseconds;
      $(QuicView.STATUS_MARK_QUIC_BROKEN).textContent =
          !!quicInfo.mark_quic_broken_when_network_blackholes;
      $(QuicView.STATUS_DO_NOT_MARK_AS_BROKEN).textContent =
          !!quicInfo.do_not_mark_as_broken_on_network_change;
      $(QuicView.STATUS_RETRY_WITHOUT_ALT).textContent =
          !!quicInfo.retry_without_alt_svc_on_quic_errors;
      $(QuicView.STATUS_DO_NOT_FRAGMENT).textContent =
          !!quicInfo.do_not_fragment;
      $(QuicView.STATUS_ALLOW_SERVER_MIGRATION).textContent =
          !!quicInfo.allow_server_migration;
      $(QuicView.STATUS_MIGRATE_SESSIONS_EARLY_V2).textContent =
          !!quicInfo.migrate_sessions_early_v2;
      $(QuicView.STATUS_MIGRATE_SESSION_ON_NETWORK_CHANGE_V2).textContent =
          !!quicInfo.migrate_sessions_on_network_change_v2;
      $(QuicView.STATUS_RETRANSMITTABLE_ON_WIRE_TIMEOUT).textContent =
          !!quicInfo.retransmittable_on_wire_timeout_milliseconds;
      $(QuicView.STATUS_DISABLE_BIDIRECTIONAL_STREAMS).textContent =
          !!quicInfo.disable_bidirectional_streams;
      $(QuicView.STATUS_RACE_CERT_VERIFICATION).textContent =
          !!quicInfo.race_cert_verification;
      $(QuicView.STATUS_RACE_STALE_DNS_ON_CONNECTION).textContent =
          !!quicInfo.race_stale_dns_on_connection;
      $(QuicView.STATUS_ESTIMATE_INITIAL_RTT).textContent =
          !!quicInfo.estimate_initial_rtt;
      $(QuicView.STATUS_FORCE_HOL_BLOCKING).textContent =
          !!quicInfo.force_hol_blocking;
      $(QuicView.STATUS_MAX_SERVER_CONFIGS_STORED_IN_PROPERTIES).textContent =
          quicInfo.max_server_configs_stored_in_properties;
      $(QuicView.STATUS_ORIGINS_TO_FORCE_QUIC_ON).textContent =
          quicInfo.origins_to_force_quic_on;
      $(QuicView.STATUS_SERVER_PUSH_CANCELLATION).textContent =
          !!quicInfo.server_push_cancellation;

      var sessions = quicInfo.sessions;

      var hasSessions = sessions && sessions.length > 0;

      setNodeDisplay($(QuicView.SESSION_INFO_CONTENT_ID), hasSessions);
      setNodeDisplay($(QuicView.SESSION_INFO_NO_CONTENT_ID), !hasSessions);

      var tbody = $(QuicView.SESSION_INFO_TBODY_ID);
      tbody.innerHTML = '';

      // Fill in the sessions info table.
      for (var i = 0; i < sessions.length; ++i) {
        var q = sessions[i];
        var tr = addNode(tbody, 'tr');

        addNodeWithText(tr, 'td', q.aliases ? q.aliases.join(' ') : '');
        addNodeWithText(tr, 'td', q.version);
        addNodeWithText(tr, 'td', q.peer_address);

        var connectionUIDCell = addNode(tr, 'td');
        var a = addNode(connectionUIDCell, 'a');
        a.href = '#events&q=type:QUIC_SESSION%20' + q.connection_id;
        a.textContent = q.connection_id;

        addNodeWithText(tr, 'td', q.open_streams);

        addNodeWithText(tr, 'td',
            q.active_streams && q.active_streams.length > 0 ?
            q.active_streams.join(', ') : 'None');

        addNodeWithText(tr, 'td', q.total_streams);
        addNodeWithText(tr, 'td', q.packets_sent);
        addNodeWithText(tr, 'td', q.packets_lost);
        addNodeWithText(tr, 'td', q.packets_received);
        addNodeWithText(tr, 'td', q.connected);
      }

      return true;
    },
  };

  return QuicView;
})();

