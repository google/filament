// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * This view displays a summary of the current Reporting cache, including the
 * configuration headers received for Reporting-enabled origins, and any queued
 * reports that are waiting to be uploaded.
 */
'use strict';

var ReportingView = (function() {
  'use strict';

  // We inherit from DivView.
  var superClass = DivView;

  /**
   * @constructor
   */
  function ReportingView() {
    assertFirstConstructorCall(ReportingView);

    // Call superclass's constructor.
    superClass.call(this, ReportingView.MAIN_BOX_ID);

    g_browser.addReportingInfoObserver(this, true);
  }

  ReportingView.TAB_ID = 'tab-handle-reporting';
  ReportingView.TAB_NAME = 'Reporting';
  ReportingView.TAB_HASH = '#reporting';

  // IDs for special HTML elements in reporting_view.html
  ReportingView.MAIN_BOX_ID = 'reporting-view-tab-content';

  ReportingView.DISABLED_BOX_ID = 'reporting-view-disabled-content';
  ReportingView.ENABLED_BOX_ID = 'reporting-view-enabled-content';

  ReportingView.CLIENTS_EMPTY_ID = 'reporting-view-clients-empty';
  ReportingView.CLIENTS_TABLE_ID = 'reporting-view-clients-table';
  ReportingView.CLIENTS_TBODY_ID = 'reporting-view-clients-tbody';
  ReportingView.REPORTS_EMPTY_ID = 'reporting-view-reports-empty';
  ReportingView.REPORTS_TABLE_ID = 'reporting-view-reports-table';
  ReportingView.REPORTS_TBODY_ID = 'reporting-view-reports-tbody';

  ReportingView.NEL_POLICIES_DISABLED_ID =
      'reporting-view-nel-policies-disabled';
  ReportingView.NEL_POLICIES_EMPTY_ID = 'reporting-view-nel-policies-empty';
  ReportingView.NEL_POLICIES_TABLE_ID = 'reporting-view-nel-policies-table';
  ReportingView.NEL_POLICIES_TBODY_ID = 'reporting-view-nel-policies-tbody';

  cr.addSingletonGetter(ReportingView);

  ReportingView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    onLoadLogFinish: function(data) {
      return this.onReportingInfoChanged(data.reportingInfo);
    },

    onReportingInfoChanged: function(reportingInfo) {
      if (!isObject_(reportingInfo))
        return false;

      var enabled = !!reportingInfo.reportingEnabled;
      setNodeDisplay($(ReportingView.DISABLED_BOX_ID), !enabled);
      setNodeDisplay($(ReportingView.ENABLED_BOX_ID), enabled);
      if (!enabled)
        return true;

      displayReportDetail_(ensureArray_(reportingInfo.reports));
      displayClientDetail_(ensureArray_(reportingInfo.clients));
      displayNELPolicyDetail_(reportingInfo.networkErrorLogging);
      return true;
    },
  };

  /**
   * Displays information about each queued report in the Reporting cache.
   * REQUIRES: |reportList| must be an array
   */
  function displayReportDetail_(reportList) {
    // Clear the existing content.
    $(ReportingView.REPORTS_TBODY_ID).innerHTML = '';

    var empty = reportList.length == 0;
    setNodeDisplay($(ReportingView.REPORTS_EMPTY_ID), empty);
    setNodeDisplay($(ReportingView.REPORTS_TABLE_ID), !empty);
    if (empty)
      return;

    for (var i = 0; i < reportList.length; ++i) {
      var report = ensureObject_(reportList[i]);
      var tr = addNode($(ReportingView.REPORTS_TBODY_ID), 'tr');

      var queuedNode = addNode(tr, 'td');
      if (report.queued != undefined) {
        var queuedDate = timeutil.convertTimeTicksToDate(report.queued);
        timeutil.addNodeWithDate(queuedNode, queuedDate);
      }

      addNodeWithText(tr, 'td', report.url);

      var statusNode = addNode(tr, 'td');
      addTextNode(statusNode, report.status);
      addTextNode(statusNode, ' (' + report.group);
      if (report.depth !== undefined && report.depth > 0)
        addTextNode(statusNode, ', depth: ' + report.depth);
      if (report.attempts !== undefined && report.attempts > 0)
        addTextNode(statusNode, ', attempts: ' + report.attempts);
      addTextNode(statusNode, ')');

      addNodeWithText(tr, 'td', report.type);

      var networkKey;
      if ('network_anonymization_key' in report) {
        networkKey = report.network_anonymization_key;
      } else {
        networkKey = report.network_isolation_key;
        // Around M108 the network_isolation_key changed to be named
        // network_anonymization_key, so we do this check for backwards
        // compatibility.
      }
      addNodeWithText(tr, 'td', networkKey);

      var contentNode = addNode(tr, 'td');
      if (report.type == 'network-error')
        displayNetworkErrorContent_(contentNode, report);
      else
        displayGenericReportContent_(contentNode, report);
    }
  }

  /**
   * Adds nodes to the "content" cell for a report that allow you to show a
   * summary as well as collapsable detail.  We will add a clickable button that
   * toggles between showing and hiding the detail; its label will be `showText`
   * when the detail is hidden, and `hideText` when it's visible.
   *
   * The result is an object containing `summary` and `detail` nodes.  You can
   * add whatever content you want to each of these nodes.  The summary should
   * be a one-liner, and will be a <span>.  The detail can be as large as you
   * want, and will be a <div>.
   */
  function addContentSections_(contentNode, showText, hideText) {
    var sections = {};

    sections.summary = addNode(contentNode, 'span');
    sections.summary.classList.add('reporting-content-summary');

    var button = addNode(contentNode, 'span');
    button.classList.add('reporting-content-expand-button');
    addTextNode(button, showText);
    button.onclick = function() {
      toggleNodeDisplay(sections.detail);
      button.textContent =
          getNodeDisplay(sections.detail) ? hideText : showText;
    };

    sections.detail = addNode(contentNode, 'div');
    sections.detail.classList.add('reporting-content-detail');
    setNodeDisplay(sections.detail, false);

    return sections;
  }

  /**
   * Displays format-specific detail for Network Error Logging reports.
   * REQUIRES: |report| must be an object
   */
  function displayNetworkErrorContent_(contentNode, report) {
    var contentSections =
        addContentSections_(contentNode, 'Show raw report', 'Hide raw report');

    var body = ensureObject_(report.body);
    addTextNode(contentSections.summary, body.type);
    // Only show the status code if it's present and not 0.
    if (body.status_code)
      addTextNode(
          contentSections.summary, ' (' + report.body.status_code + ')');

    addNodeWithText(
        contentSections.detail, 'pre', JSON.stringify(report, null, '  '));
  }

  /**
   * Displays a generic content cell for reports whose type we don't know how to
   * render something specific for.
   * REQUIRES: |report| must be an object
   */
  function displayGenericReportContent_(contentNode, report) {
    var contentSections =
        addContentSections_(contentNode, 'Show raw report', 'Hide raw report');
    addNodeWithText(
        contentSections.detail, 'pre', JSON.stringify(report, null, '  '));
  }

  /**
   * Displays information about each origin that has provided Reporting headers.
   * REQUIRES: |clientList| must be an array
   */
  function displayClientDetail_(clientList) {
    // Clear the existing content.
    $(ReportingView.CLIENTS_TBODY_ID).innerHTML = '';

    var empty = clientList.length == 0;
    setNodeDisplay($(ReportingView.CLIENTS_EMPTY_ID), empty);
    setNodeDisplay($(ReportingView.CLIENTS_TABLE_ID), !empty);
    if (empty)
      return;

    for (var i = 0; i < clientList.length; ++i) {
      var client = ensureObject_(clientList[i]);
      var groups = ensureArray_(client.groups);
      if (groups.length == 0)
        continue;

      // Calculate the total number of endpoints for this client, so that we can
      // rowspan its origin and NIK cells.
      var clientHeight = 0;
      for (var j = 0; j < groups.length; ++j) {
        var group = ensureObject_(groups[j]);
        var endpoints = ensureArray_(group.endpoints);
        clientHeight += group.endpoints.length;
      }
      if (clientHeight == 0)
        continue;

      for (var j = 0; j < groups.length; ++j) {
        var group = ensureObject_(groups[j]);
        var endpoints = ensureArray_(group.endpoints);
        for (var k = 0; k < endpoints.length; ++k) {
          var endpoint = ensureObject_(endpoints[k]);
          var tr = addNode($(ReportingView.CLIENTS_TBODY_ID), 'tr');

          if (j == 0 && k == 0) {
            var originNode = addNode(tr, 'td');
            originNode.setAttribute('rowspan', clientHeight);
            addTextNode(originNode, client.origin);
            var nikNode = addNode(tr, 'td');
            nikNode.setAttribute('rowspan', clientHeight);
            var networkKey;
            if ('network_anonymization_key' in client) {
              networkKey = client.network_anonymization_key;
            } else {
              networkKey = client.network_isolation_key;
              // Around M108 the network_isolation_key changed to be named
              // network_anonymization_key, so we do this check for backwards
              // compatibility.
            }
            addTextNode(nikNode, networkKey);
          }

          if (k == 0) {
            var groupNode = addNode(tr, 'td');
            groupNode.setAttribute('rowspan', group.endpoints.length);
            addTextNode(groupNode, group.name);

            var subdomainsNode = addNode(tr, 'td');
            subdomainsNode.classList.add('reporting-centered');
            subdomainsNode.setAttribute('rowspan', group.endpoints.length);
            addTextNode(
                subdomainsNode, !!group.includeSubdomains ? 'yes' : 'no');

            var expiresNode = addNode(tr, 'td');
            expiresNode.setAttribute('rowspan', group.endpoints.length);
            if (group.expires !== undefined) {
              var expiresDate = timeutil.convertTimeTicksToDate(group.expires);
              timeutil.addNodeWithDate(expiresNode, expiresDate);
              if (expired_(expiresDate)) {
                var expiredSpan = addNode(expiresNode, 'span');
                expiredSpan.classList.add('warning-text');
                addTextNode(expiredSpan, ' [expired]');
              }
            }
          }

          var endpointNode = addNode(tr, 'td');
          addTextNode(endpointNode, endpoint.url);

          var priorityNode = addNode(tr, 'td');
          priorityNode.classList.add('reporting-centered');
          addTextNode(priorityNode, valueOrDefault_(endpoint.priority, 0));

          var weightNode = addNode(tr, 'td');
          weightNode.classList.add('reporting-centered');
          addTextNode(weightNode, valueOrDefault_(endpoint.weight, 1));

          addUploadCount_(tr, ensureObject_(endpoint.successful));
          addUploadCount_(tr, ensureObject_(endpoint.failed));
        }
      }
    }
  }

  /**
   * Adds an upload count cell to the client details table.
   * REQUIRES: |counts| must be an object
   */
  function addUploadCount_(tr, counts) {
    var node = addNode(tr, 'td');
    node.classList.add('reporting-centered');
    var uploads = valueOrDefault_(counts.uploads, 0);
    var reports = valueOrDefault_(counts.reports, 0);
    if (uploads == 0 && reports == 0) {
      addTextNode(node, '-');
    } else {
      addTextNode(node, uploads + ' (' + reports + ')');
    }
  }

  /**
   * Displays information about each origin that has provided NEL headers.
   */
  function displayNELPolicyDetail_(nelInfo) {
    // Clear the existing content.
    $(ReportingView.NEL_POLICIES_TBODY_ID).innerHTML = '';

    var disabled = (nelInfo === undefined);
    setNodeDisplay($(ReportingView.NEL_POLICIES_DISABLED_ID), disabled);
    if (disabled) {
      setNodeDisplay($(ReportingView.NEL_POLICIES_EMPTY_ID), false);
      setNodeDisplay($(ReportingView.NEL_POLICIES_TABLE_ID), false);
      return;
    }

    nelInfo = ensureObject_(nelInfo);
    var policies = ensureArray_(nelInfo.originPolicies);
    var empty = policies.length == 0;
    setNodeDisplay($(ReportingView.NEL_POLICIES_EMPTY_ID), empty);
    setNodeDisplay($(ReportingView.NEL_POLICIES_TABLE_ID), !empty);
    if (empty)
      return;

    for (var i = 0; i < policies.length; ++i) {
      var policy = ensureObject_(policies[i]);
      var tr = addNode($(ReportingView.NEL_POLICIES_TBODY_ID), 'tr');

      addNodeWithText(tr, 'td', policy.origin);

      var subdomainsNode = addNode(tr, 'td');
      subdomainsNode.classList.add('reporting-centered');
      addTextNode(subdomainsNode, !!policy.includeSubdomains ? 'yes' : 'no');

      var expiresNode = addNode(tr, 'td');
      if (policy.expires !== undefined) {
        var expiresDate = timeutil.convertTimeTicksToDate(policy.expires);
        timeutil.addNodeWithDate(expiresNode, expiresDate);
        if (expired_(expiresDate)) {
          var expiredSpan = addNode(expiresNode, 'span');
          expiredSpan.classList.add('warning-text');
          addTextNode(expiredSpan, ' [expired]');
        }
      }

      addNodeWithText(tr, 'td', policy.reportTo);
      if ('NetworkAnonymizationKey' in policy) {
        // In M108 this key was changed as part of the network state
        // partitioning project.
        addNodeWithText(tr, 'td', policy.NetworkAnonymizationKey);
      } else {
        addNodeWithText(tr, 'td', policy.networkIsolationKey);
      }

      var successFractionNode = addNode(tr, 'td');
      successFractionNode.classList.add('reporting-right-justified');
      addTextNode(successFractionNode, percent_(policy.successFraction));

      var failureFractionNode = addNode(tr, 'td');
      failureFractionNode.classList.add('reporting-right-justified');
      addTextNode(failureFractionNode, percent_(policy.failureFraction));
    }
  }

  /**
   * Returns whether an expiry timestamp has expired.  If we're viewing live
   * data, uses the actual current time to determine whether it's expired.  If
   * we're viewing data from a saved log file, uses the timestamp when the file
   * was recorded.
   *
   * @param {Date} expiry An expiry time
   */
  function expired_(expiry) {
    var now;
    if (MainView.isViewingLoadedLog()) {
      now = new Date(ClientInfo.numericDate);
    } else {
      now = new Date();
    }
    return expiry < now;
  }

  /**
   * Formats a float fraction as a percentage.
   */
  function percent_(fraction) {
    return (valueOrDefault_(fraction, 0) * 100).toFixed(2) + '%';
  }

  function isObject_(value) {
    return value && typeof(value) === 'object';
  }

  function isArray_(value) {
    return value !== undefined && value instanceof Array;
  }

  function ensureObject_(value) {
    if (isObject_(value))
      return value;
    return {};
  }

  function ensureArray_(value) {
    if (isArray_(value))
      return value;
    return [];
  }

  function valueOrDefault_(value, defaultValue) {
    if (value != undefined)
      return value;
    return defaultValue;
  }

  return ReportingView;
})();
