// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays information on the host resolver:
 *
 *   - Shows the default address family.
 *   - Shows the current host cache contents.
 *   - Has a button to clear the host cache.
 *   - Shows the parameters used to construct the host cache (capacity, ttl).
 */

'use strict';

// TODO(mmenke):  Add links for each address entry to the corresponding NetLog
//                source.  This could either be done by adding NetLog source ids
//                to cache entries, or tracking sources based on their type and
//                description.  Former is simpler, latter may be useful
//                elsewhere as well.
var DnsView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   *  @constructor
   */
  function DnsView() {
    assertFirstConstructorCall(DnsView);

    // Call superclass's constructor.
    superClass.call(this, DnsView.MAIN_BOX_ID);

    // Register to receive changes to the host resolver info.
    g_browser.addHostResolverInfoObserver(this, false);
  }

  DnsView.TAB_ID = 'tab-handle-dns';
  DnsView.TAB_NAME = 'DNS';
  DnsView.TAB_HASH = '#dns';

  // IDs for special HTML elements in dns_view.html
  DnsView.MAIN_BOX_ID = 'dns-view-tab-content';

  DnsView.INTERNAL_DNS_ENABLED_FOR_INSECURE_SPAN_ID =
      'dns-view-internal-dns-enabled-for-insecure';
  DnsView.INTERNAL_DNS_ENABLED_FOR_SECURE_SPAN_ID =
      'dns-view-internal-dns-enabled-for-secure';
  DnsView.INTERNAL_DNS_CONFIG_TBODY_ID = 'dns-view-internal-dns-config-tbody';
  DnsView.INTERNAL_DISABLED_DOH_PROVIDERS_UL_ID =
      'dns-view-internal-disabled-doh-providers';

  DnsView.CAPACITY_SPAN_ID = 'dns-view-cache-capacity';

  DnsView.ACTIVE_SPAN_ID = 'dns-view-cache-active';
  DnsView.EXPIRED_SPAN_ID = 'dns-view-cache-expired';
  DnsView.NETWORK_SPAN_ID = 'dns-view-network-changes';
  DnsView.CACHE_TBODY_ID = 'dns-view-cache-tbody';

  cr.addSingletonGetter(DnsView);

  DnsView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onLoadLogFinish: function(data) {
      return this.onHostResolverInfoChanged(
          data.hostResolverInfo, data.dohProvidersDisabledDueToFeature);
    },

    onHostResolverInfoChanged: function(
        hostResolverInfo, dohProvidersDisabledDueToFeature) {
      // Clear the existing values.
      $(DnsView.CAPACITY_SPAN_ID).innerHTML = '';
      $(DnsView.CACHE_TBODY_ID).innerHTML = '';
      $(DnsView.ACTIVE_SPAN_ID).innerHTML = '0';
      $(DnsView.EXPIRED_SPAN_ID).innerHTML = '0';
      $(DnsView.NETWORK_SPAN_ID).innerHTML = '0';

      // Update fields containing async DNS configuration information.
      displayAsyncDnsConfig_(
          hostResolverInfo, dohProvidersDisabledDueToFeature);

      // No info.
      if (!hostResolverInfo || !hostResolverInfo.cache)
        return false;

      // Fill in the basic cache information.
      var hostResolverCache = hostResolverInfo.cache;
      $(DnsView.CAPACITY_SPAN_ID).innerText = hostResolverCache.capacity;
      $(DnsView.NETWORK_SPAN_ID).innerText = hostResolverCache.network_changes;

      var expiredEntries = 0;
      // Date the cache was logged.  This will be either now, when actively
      // logging data, or the date the log dump was created.
      var logDate;
      if (MainView.isViewingLoadedLog()) {
        logDate = new Date(ClientInfo.numericDate);
      } else {
        logDate = new Date();
      }

      // Fill in the cache contents table.
      for (const e of hostResolverCache.entries) {
        var tr = addNode($(DnsView.CACHE_TBODY_ID), 'tr');
        var expired = false;

        var hostnameCell = addNode(tr, 'td');
        addTextNode(hostnameCell, e.hostname);

        var familyCell = addNode(tr, 'td');
        addTextNode(familyCell, addressFamilyToString(e.address_family));

        var addressesCell = addNode(tr, 'td');

        if (e.net_error != undefined) {
          var errorText = e.error + ' (' + netErrorToString(e.error) + ')';
          var errorNode = addTextNode(addressesCell, 'error: ' + errorText);
          addressesCell.classList.add('warning-text');
        } else {
          // Concatenate the legacy `addresses` and `ip_endpoints`.
          let addresses = [];
          if ('addresses' in e)
            addresses = addresses.concat(e.addresses);
          if ('ip_endpoints' in e)
            addresses = addresses.concat(e.ip_endpoints.map(JSON.stringify));
          if (addresses.length > 0)
            addListToNode_(addNode(addressesCell, 'div'), addresses);
        }

        var ttlCell = addNode(tr, 'td');
        addTextNode(ttlCell, e.ttl);

        var expiresDate = timeutil.convertTimeTicksToDate(e.expiration);
        var expiresCell = addNode(tr, 'td');
        timeutil.addNodeWithDate(expiresCell, expiresDate);
        if (logDate > timeutil.convertTimeTicksToDate(e.expiration)) {
          expired = true;
          var expiredSpan = addNode(expiresCell, 'span');
          expiredSpan.classList.add('warning-text');
          addTextNode(expiredSpan, ' [Expired]');
        }

        var nikCell = addNode(tr, 'td');
        var networkKey;
        if ('network_anonymization_key' in e) {
          networkKey = e.network_anonymization_key;
        } else {
          // Versions prior to M84 used lists instead of strings for logged
          // NIKs.
          networkKey = '' + e.network_isolation_key;
          // Around M108 the network_isolation_key changed to be named
          // network_anonymization_key, so we do this check for backwards
          // compatibility.
        }
        addTextNode(nikCell, networkKey);

        // HostCache keeps track of how many network changes have happened since
        // it was created, and entries store what that number was at the time
        // they were created. If more network changes have happened since an
        // entry was created, the entry is expired.
        var networkChangesCell = addNode(tr, 'td');
        addTextNode(networkChangesCell, e.network_changes);
        if (e.network_changes < hostResolverCache.network_changes) {
          expired = true;
          var expiredSpan = addNode(networkChangesCell, 'span');
          expiredSpan.classList.add('warning-text');
          addTextNode(expiredSpan, ' [Expired]');
        }

        if (expired) {
          expiredEntries++;
        }
      }

      $(DnsView.ACTIVE_SPAN_ID).innerText =
          hostResolverCache.entries.length - expiredEntries;
      $(DnsView.EXPIRED_SPAN_ID).innerText = expiredEntries;
      return true;
    },
  };

  /**
   * Displays information corresponding to the current async DNS configuration.
   * @param {Object} hostResolverInfo The host resolver information.
   */
  function displayAsyncDnsConfig_(
      hostResolverInfo, dohProvidersDisabledDueToFeature) {
    // Clear the existing values.
    $(DnsView.INTERNAL_DISABLED_DOH_PROVIDERS_UL_ID).innerHTML = '';
    $(DnsView.INTERNAL_DNS_CONFIG_TBODY_ID).innerHTML = '';

    // Determine whether the async resolver is enabled for both Do53 and DoH.
    // Update the display accordingly.
    const enabled_for_insecure =
        hostResolverInfo && hostResolverInfo.dns_config &&
        hostResolverInfo.dns_config.can_use_insecure_dns_transactions;
    const enabled_for_secure =
        hostResolverInfo && hostResolverInfo.dns_config &&
        hostResolverInfo.dns_config.can_use_secure_dns_transactions;
    $(DnsView.INTERNAL_DNS_ENABLED_FOR_INSECURE_SPAN_ID).innerText =
        enabled_for_insecure;
    $(DnsView.INTERNAL_DNS_ENABLED_FOR_SECURE_SPAN_ID).innerText =
        enabled_for_secure;

    // Show the list of disabled DoH providers.
    if (dohProvidersDisabledDueToFeature) {
      for (let disabledProvider of dohProvidersDisabledDueToFeature) {
        addNodeWithText(
            $(DnsView.INTERNAL_DISABLED_DOH_PROVIDERS_UL_ID), 'li',
            disabledProvider);
      }
    }

    // Attempt to display the async resolver's DNS configuration. It may be
    // relevant if there were any DoH queries.
    const dnsConfig = hostResolverInfo && hostResolverInfo.dns_config;
    if (!dnsConfig)
      return;

    // Decide the display order for the keys of `dnsConfig`.
    let keys = Object.keys(dnsConfig).sort();
    const keysToDrop = [
      // Nameservers will be re-added at the front later.
      'nameservers',
      // These keys have already been rendered outside of the table.
      'can_use_insecure_dns_transactions',
      'can_use_secure_dns_transactions',
    ];
    keys = keys.filter((k) => !keysToDrop.includes(k));
    keys.unshift('nameservers');  // Push 'nameservers' to the front.

    // Add selected keys from `dnsConfig` to the table.
    for (const key of keys) {
      const tr = addNode($(DnsView.INTERNAL_DNS_CONFIG_TBODY_ID), 'tr');
      addNodeWithText(tr, 'th', key);
      const td = addNode(tr, 'td');

      // For lists, display each list entry on a separate line.
      if (Array.isArray(dnsConfig[key])) {
        const strings = dnsConfig[key].map(JSON.stringify);
        addListToNode_(td, strings);
        continue;
      }

      addTextNode(td, dnsConfig[key]);
    }
  }

  /**
   * Takes a last of strings and adds them all to a DOM node, displaying them
   * on separate lines.
   * @param {DomNode} node The parent node.
   * @param {Array<string>} list List of strings to add to the node.
   */
  function addListToNode_(node, list) {
    for (var i = 0; i < list.length; ++i)
      addNodeWithText(node, 'div', list[i]);
  }

  return DnsView;
})();

