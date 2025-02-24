// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays information on the proxy setup:
 *
 *   - Shows the current proxy settings.
 *   - Has a button to reload these settings.
 *   - Shows the list of proxy hostnames that are cached as "bad".
 *   - Has a button to clear the cached bad proxies.
 */
var ProxyView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function ProxyView() {
    assertFirstConstructorCall(ProxyView);

    // Call superclass's constructor.
    superClass.call(this, ProxyView.MAIN_BOX_ID);

    // Register to receive proxy information as it changes.
    g_browser.addProxySettingsObserver(this, true);
    g_browser.addBadProxiesObserver(this, true);
  }

  ProxyView.TAB_ID = 'tab-handle-proxy';
  ProxyView.TAB_NAME = 'Proxy';
  ProxyView.TAB_HASH = '#proxy';

  // IDs for special HTML elements in proxy_view.html
  ProxyView.MAIN_BOX_ID = 'proxy-view-tab-content';
  ProxyView.ORIGINAL_SETTINGS_DIV_ID = 'proxy-view-original-settings';
  ProxyView.EFFECTIVE_SETTINGS_DIV_ID = 'proxy-view-effective-settings';
  ProxyView.ORIGINAL_CONTENT_DIV_ID = 'proxy-view-original-content';
  ProxyView.EFFECTIVE_CONTENT_DIV_ID = 'proxy-view-effective-content';
  ProxyView.BAD_PROXIES_DIV_ID = 'proxy-view-bad-proxies-div';
  ProxyView.BAD_PROXIES_TBODY_ID = 'proxy-view-bad-proxies-tbody';
  ProxyView.SOCKS_HINTS_DIV_ID = 'proxy-view-socks-hints';
  ProxyView.SOCKS_HINTS_FLAG_DIV_ID = 'proxy-view-socks-hints-flag';

  cr.addSingletonGetter(ProxyView);

  ProxyView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onLoadLogFinish: function(data) {
      return this.onProxySettingsChanged(data.proxySettings) &&
          this.onBadProxiesChanged(data.badProxies);
    },

    onProxySettingsChanged: function(proxySettings) {
      $(ProxyView.ORIGINAL_SETTINGS_DIV_ID).innerHTML = '';
      $(ProxyView.EFFECTIVE_SETTINGS_DIV_ID).innerHTML = '';
      this.updateSocksHints_(null);

      if (!proxySettings)
        return false;

      // Both |original| and |effective| are dictionaries describing the
      // settings.
      var original = proxySettings.original;
      var effective = proxySettings.effective;

      var originalStr = proxySettingsToString(original);
      var effectiveStr = proxySettingsToString(effective);

      setNodeDisplay(
          $(ProxyView.ORIGINAL_CONTENT_DIV_ID), originalStr != effectiveStr);

      $(ProxyView.ORIGINAL_SETTINGS_DIV_ID).innerText = originalStr;
      $(ProxyView.EFFECTIVE_SETTINGS_DIV_ID).innerText = effectiveStr;

      this.updateSocksHints_(effective);

      return true;
    },

    onBadProxiesChanged: function(badProxies) {
      $(ProxyView.BAD_PROXIES_TBODY_ID).innerHTML = '';
      setNodeDisplay(
          $(ProxyView.BAD_PROXIES_DIV_ID), badProxies && badProxies.length > 0);

      if (!badProxies)
        return false;

      // Add a table row for each bad proxy entry.
      for (var i = 0; i < badProxies.length; ++i) {
        var entry = badProxies[i];
        var badUntilDate = timeutil.convertTimeTicksToDate(entry.bad_until);

        var tr = addNode($(ProxyView.BAD_PROXIES_TBODY_ID), 'tr');

        var nameCell = addNode(tr, 'td');
        var badUntilCell = addNode(tr, 'td');

        addTextNode(nameCell, entry.proxy_uri);
        timeutil.addNodeWithDate(badUntilCell, badUntilDate);
      }
      return true;
    },

    updateSocksHints_: function(proxySettings) {
      setNodeDisplay($(ProxyView.SOCKS_HINTS_DIV_ID), false);

      if (!proxySettings)
        return;

      var socksProxy = getSingleSocks5Proxy_(proxySettings.single_proxy);
      if (!socksProxy)
        return;

      // Suggest a recommended --host-resolver-rules.
      // NOTE: This does not compensate for any proxy bypass rules. If the
      // proxy settings include proxy bypasses the user may need to expand the
      // exclusions for host resolving.
      var hostResolverRules = 'MAP * ~NOTFOUND , EXCLUDE ' + socksProxy.host;
      var hostResolverRulesFlag =
          '--host-resolver-rules="' + hostResolverRules + '"';

      // TODO(eroman): On Linux the ClientInfo.command_line is wrong in that it
      // doesn't include any quotes around the parameters. This means the
      // string search above is going to fail :(
      if (ClientInfo.command_line &&
          ClientInfo.command_line.indexOf(hostResolverRulesFlag) != -1) {
        // Chrome is already using the suggested resolver rules.
        return;
      }

      $(ProxyView.SOCKS_HINTS_FLAG_DIV_ID).innerText = hostResolverRulesFlag;
      setNodeDisplay($(ProxyView.SOCKS_HINTS_DIV_ID), true);
    }
  };

  function getSingleSocks5Proxy_(proxyList) {
    var proxyString;
    if (typeof proxyList == 'string') {
      // Older versions of Chrome passed single_proxy as a string.
      // TODO(eroman): This behavior changed in M27. Support for older logs can
      //               safely be removed circa M29.
      proxyString = proxyList;
    } else if (Array.isArray(proxyList) && proxyList.length == 1) {
      proxyString = proxyList[0];
    } else {
      return null;
    }

    var pattern = /^socks5:\/\/(.*)$/;
    var matches = pattern.exec(proxyString);

    if (!matches)
      return null;

    var hostPortString = matches[1];

    matches = /^(.*):(\d+)$/.exec(hostPortString);
    if (!matches)
      return null;

    var result = {host: matches[1], port: matches[2]};

    // Strip brackets off of IPv6 literals.
    matches = /^\[(.*)\]$/.exec(result.host);
    if (matches)
      result.host = matches[1];

    return result;
  }

  return ProxyView;
})();

