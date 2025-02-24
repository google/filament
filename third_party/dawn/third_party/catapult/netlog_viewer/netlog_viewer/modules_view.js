// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays information on installed Chrome extensions / apps as well
 * as Winsock layered service providers and namespace providers.
 *
 * For each layered service provider, shows the name, dll, and type
 * information.  For each namespace provider, shows the name and
 * whether or not it's active.
 */
var ModulesView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function ModulesView() {
    assertFirstConstructorCall(ModulesView);

    // Call superclass's constructor.
    superClass.call(this, ModulesView.MAIN_BOX_ID);

    this.serviceProvidersTbody_ = $(ModulesView.SERVICE_PROVIDERS_TBODY_ID);
    this.namespaceProvidersTbody_ = $(ModulesView.NAMESPACE_PROVIDERS_TBODY_ID);

    g_browser.addServiceProvidersObserver(this, false);
    g_browser.addExtensionInfoObserver(this, true);
  }

  ModulesView.TAB_ID = 'tab-handle-modules';
  ModulesView.TAB_NAME = 'Modules';
  ModulesView.TAB_HASH = '#modules';

  // IDs for special HTML elements in modules_view.html.
  ModulesView.MAIN_BOX_ID = 'modules-view-tab-content';
  ModulesView.EXTENSION_INFO_ID = 'modules-view-extension-info';
  ModulesView.EXTENSION_INFO_UNAVAILABLE_ID =
      'modules-view-extension-info-unavailable';
  ModulesView.EXTENSION_INFO_NO_CONTENT_ID =
      'modules-view-extension-info-no-content';
  ModulesView.EXTENSION_INFO_CONTENT_ID =
      'modules-view-extension-info-content';
  ModulesView.EXTENSION_INFO_TBODY_ID =
      'modules-view-extension-info-tbody';
  ModulesView.WINDOWS_SERVICE_PROVIDERS_ID =
      'modules-view-windows-service-providers';
  ModulesView.SERVICE_PROVIDERS_TBODY_ID =
      'modules-view-service-providers-tbody';
  ModulesView.NAMESPACE_PROVIDERS_TBODY_ID =
      'modules-view-namespace-providers-tbody';

  cr.addSingletonGetter(ModulesView);

  ModulesView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onLoadLogFinish: function(data) {
      // Show the tab if there are either service providers or extension info.
      var hasExtensionInfo = this.onExtensionInfoChanged(data.extensionInfo);
      var hasSpiInfo = this.onServiceProvidersChanged(data.serviceProviders);
      return hasExtensionInfo || hasSpiInfo;
    },

    onExtensionInfoChanged: function(extensionInfo) {
      setNodeDisplay($(ModulesView.EXTENSION_INFO_CONTENT_ID),
          extensionInfo && extensionInfo.length > 0);
      setNodeDisplay($(ModulesView.EXTENSION_INFO_UNAVAILABLE_ID),
          !extensionInfo);
      setNodeDisplay($(ModulesView.EXTENSION_INFO_NO_CONTENT_ID),
          extensionInfo && extensionInfo.length == 0);

      var tbodyExtension = $(ModulesView.EXTENSION_INFO_TBODY_ID);
      tbodyExtension.innerHTML = '';

      if (extensionInfo && extensionInfo.length > 0) {
        // Fill in the extensions table.
        for (var i = 0; i < extensionInfo.length; ++i) {
          var e = extensionInfo[i];
          var tr = addNode(tbodyExtension, 'tr');
          tr.className = (e.enabled ? 'enabled' : '');

          addNodeWithText(tr, 'td', e.id);
          addNodeWithText(tr, 'td', e.packagedApp);
          addNodeWithText(tr, 'td', e.enabled);
          addNodeWithText(tr, 'td', e.name);
          addNodeWithText(tr, 'td', e.version);
          addNodeWithText(tr, 'td', e.description);
        }
      }

      return !!extensionInfo;
    },

    onServiceProvidersChanged: function(serviceProviders) {
      setNodeDisplay($(ModulesView.WINDOWS_SERVICE_PROVIDERS_ID),
          serviceProviders);
      if (serviceProviders) {
        var tbodyService = $(ModulesView.SERVICE_PROVIDERS_TBODY_ID);
        tbodyService.innerHTML = '';

        // Fill in the service providers table.
        for (var i = 0; i < serviceProviders.service_providers.length; ++i) {
          var s = serviceProviders.service_providers[i];
          var tr = addNode(tbodyService, 'tr');

          addNodeWithText(tr, 'td', s.name);
          addNodeWithText(tr, 'td', s.version);
          addNodeWithText(tr, 'td',
              ModulesView.getLayeredServiceProviderType(s));
          addNodeWithText(tr, 'td',
              ModulesView.getLayeredServiceProviderSocketType(s));
          addNodeWithText(tr, 'td',
              ModulesView.getLayeredServiceProviderProtocolType(s));
        }

        var tbodyNamespace = $(ModulesView.NAMESPACE_PROVIDERS_TBODY_ID);
        tbodyNamespace.innerHTML = '';

        // Fill in the namespace providers table.
        for (var i = 0; i < serviceProviders.namespace_providers.length; ++i) {
          var n = serviceProviders.namespace_providers[i];
          var tr = addNode(tbodyNamespace, 'tr');

          addNodeWithText(tr, 'td', n.name);
          addNodeWithText(tr, 'td', n.version);
          addNodeWithText(tr, 'td', ModulesView.getNamespaceProviderType(n));
          addNodeWithText(tr, 'td', n.active);
        }
      }

      return !!serviceProviders;
    },
  };

  /**
   * Returns type of a layered service provider.
   */
  ModulesView.getLayeredServiceProviderType = function(serviceProvider) {
    if (serviceProvider.chain_length == 0)
      return 'Layer';
    if (serviceProvider.chain_length == 1)
      return 'Base';
    return 'Chain';
  };

  var SOCKET_TYPE = {
    '1': 'SOCK_STREAM',
    '2': 'SOCK_DGRAM',
    '3': 'SOCK_RAW',
    '4': 'SOCK_RDM',
    '5': 'SOCK_SEQPACKET'
  };

  /**
   * Returns socket type of a layered service provider as a string.
   */
  ModulesView.getLayeredServiceProviderSocketType = function(serviceProvider) {
    return tryGetValueWithKey(SOCKET_TYPE, serviceProvider.socket_type);
  };

  var PROTOCOL_TYPE = {
    '1': 'IPPROTO_ICMP',
    '6': 'IPPROTO_TCP',
    '17': 'IPPROTO_UDP',
    '58': 'IPPROTO_ICMPV6'
  };

  /**
   * Returns protocol type of a layered service provider as a string.
   */
  ModulesView.getLayeredServiceProviderProtocolType = function(
      serviceProvider) {
    return tryGetValueWithKey(PROTOCOL_TYPE, serviceProvider.socket_protocol);
  };

  var NAMESPACE_PROVIDER_PTYPE = {
    '12': 'NS_DNS',
    '15': 'NS_NLA',
    '16': 'NS_BTH',
    '32': 'NS_NTDS',
    '37': 'NS_EMAIL',
    '38': 'NS_PNRPNAME',
    '39': 'NS_PNRPCLOUD'
  };

  /**
   * Returns the type of a namespace provider as a string.
   */
  ModulesView.getNamespaceProviderType = function(namespaceProvider) {
    return tryGetValueWithKey(NAMESPACE_PROVIDER_PTYPE, namespaceProvider.type);
  };

  return ModulesView;
})();
