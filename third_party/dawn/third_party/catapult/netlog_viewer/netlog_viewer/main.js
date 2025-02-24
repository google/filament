// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

/**
 * Dictionary of constants (Initialized soon after loading by data from browser,
 * updated on load log).  The *Types dictionaries map strings to numeric IDs,
 * while the *TypeNames are the other way around.
 */
let EventType = null;
let EventTypeNames = null;
let EventPhase = null;
let EventSourceType = null;
let EventSourceTypeNames = null;
let ClientInfo = null;
let NetError = null;
let QuicError = null;
let QuicRstStreamError = null;
let LoadFlag = null;
let CertStatusFlag = null;
let CertVerifierFlags = null;
let CertVerifyFlags = null;
let CertPathBuilderDigestPolicy = null;
let LoadState = null;
let AddressFamily = null;
let DataReductionProxyBypassEventType = null;
let DataReductionProxyBypassActionType = null;

/**
 * Dictionary of all constants, used for saving log files.
 */
let Constants = null;

/**
 * Object to communicate between the renderer and the browser.
 * @type {!BrowserBridge}
 */
let g_browser = null;

/**
 * This class is the root view object of the page.  It owns all the other
 * views, and manages switching between them.  It is also responsible for
 * initializing the views and the BrowserBridge.
 */
const MainView = (function() {
  'use strict';

  // We inherit from DivView
  const superClass = DivView;

  /**
   * Main entry point. Called once the page has loaded.
   *  @constructor
   */
  function MainView() {
    assertFirstConstructorCall(MainView);

    // TODO(eroman): This element ID is not the host element.
    superClass.call(this, 'tab-list');

    if (hasTouchScreen()) {
      document.body.classList.add('touch');
    }

    // This must be initialized before the tabs, so they can register as
    // observers.
    g_browser = BrowserBridge.getInstance();

    // This must be the first constants observer, so other constants observers
    // can safely use the globals, rather than depending on walking through
    // the constants themselves.
    g_browser.addConstantsObserver(new ConstantsObserver());

    // Create the tab switcher.
    this.initTabs_();

    this.topBarView_ = TopBarView.getInstance(this);

    window.onhashchange = this.onUrlHashChange_.bind(this);

    // Select the initial view based on the current URL.
    window.onhashchange();

    // No log file loaded yet so set the status bar to that state.
    this.topBarView_.switchToSubView('loaded').setFileName(
        'No log to display.');
  }

  cr.addSingletonGetter(MainView);

  // Tracks if we're viewing a loaded log file, so views can behave
  // appropriately.  Global so safe to call during construction.
  let isViewingLoadedLog = false;

  MainView.isViewingLoadedLog = function() {
    return isViewingLoadedLog;
  };

  MainView.prototype = {
    // Inherit the superclass's methods.
    __proto__: superClass.prototype,

    // This is exposed both so the log import/export code can enumerate all the
    // tabs, and for testing.
    tabSwitcher() {
      return this.tabSwitcher_;
    },

    /**
     * Prevents receiving/sending events to/from the browser, so loaded data
     * will not be mixed with current Chrome state.  Also hides any interactive
     * HTML elements that send messages to the browser.  Cannot be undone
     * without reloading the page.  Must be called before passing loaded data
     * to the individual views.
     *
     * @param {string} opt_fileName The name of the log file that has been
     *     loaded, if we're loading a log file.
     */
    onLoadLog(opt_fileName) {
      isViewingLoadedLog = true;

      if (opt_fileName !== undefined) {
        // If there's a file name, a log file was loaded, so display the
        // file's name in the status bar.
        this.topBarView_.switchToSubView('loaded').setFileName(opt_fileName);
      }
    },

    initTabs_() {
      this.tabIdToHash_ = {};
      this.hashToTabId_ = {};

      this.tabSwitcher_ = new TabSwitcherView(this.onTabSwitched_.bind(this));

      // Helper function to add a tab given the class for a view singleton.
      const addTab = function(viewClass) {
        const tabId = viewClass.TAB_ID;
        const tabHash = viewClass.TAB_HASH;
        const tabName = viewClass.TAB_NAME;
        const view = viewClass.getInstance();

        if (!tabId || !view || !tabHash || !tabName) {
          throw Error('Invalid view class for tab');
        }

        if (tabHash.charAt(0) !== '#') {
          throw Error('Tab hashes must start with a #');
        }

        this.tabSwitcher_.addTab(tabId, view, tabName, tabHash);
        this.tabIdToHash_[tabId] = tabHash;
        this.hashToTabId_[tabHash] = tabId;
      }.bind(this);

      // Populate the main tabs.  Even tabs that don't contain information for
      // the running OS should be created, so they can load log dumps from other
      // OSes.
      addTab(ImportView);
      addTab(ProxyView);
      addTab(EventsView);
      addTab(TimelineView);
      addTab(DnsView);
      addTab(SocketsView);
      addTab(AltSvcView);
      addTab(SpdyView);
      addTab(QuicView);
      addTab(ReportingView);
      addTab(HttpCacheView);
      addTab(ModulesView);
      addTab(PrerenderView);

      // Tab links start off hidden (besides import) since a log file has not
      // been loaded yet. This must be done after all the tabs are added so
      // that the width of the tab-list div is correctly styled.
      for (const tabId in this.tabSwitcher_.getAllTabViews()) {
        if (tabId !== ImportView.TAB_ID) {
          this.tabSwitcher_.showTabLink(tabId, false);
        }
      }
    },

    /**
     * This function is called by the tab switcher when the current tab has been
     * changed. It will update the current URL to reflect the new active tab,
     * so the back can be used to return to previous view.
     */
    onTabSwitched_(oldTabId, newTabId) {
      // Change the URL to match the new tab.

      const newTabHash = this.tabIdToHash_[newTabId];
      const parsed = parseUrlHash_(window.location.hash);
      if (parsed.tabHash !== newTabHash) {
        window.location.hash = newTabHash;
      }
    },

    onUrlHashChange_() {
      const parsed = parseUrlHash_(window.location.hash);

      if (!parsed) {
        return;
      }

      if (!parsed.tabHash) {
        // Default to the import tab.
        parsed.tabHash = ImportView.TAB_HASH;
      }

      const tabId = this.hashToTabId_[parsed.tabHash];

      if (tabId) {
        this.tabSwitcher_.switchToTab(tabId);
        if (parsed.parameters) {
          const view = this.tabSwitcher_.getTabView(tabId);
          view.setParameters(parsed.parameters);
        }
      }
    },

  };

  /**
   * Takes the current hash in form of "#tab&param1=value1&param2=value2&..."
   * and parses it into a dictionary.
   *
   * Parameters and values are decoded with decodeURIComponent().
   */
  function parseUrlHash_(hash) {
    const parameters = hash.split('&');

    let tabHash = parameters[0];
    if (tabHash === '' || tabHash === '#') {
      tabHash = undefined;
    }

    // Split each string except the first around the '='.
    let paramDict = null;
    for (let i = 1; i < parameters.length; i++) {
      const paramStrings = parameters[i].split('=');
      if (paramStrings.length !== 2) {
        continue;
      }
      if (paramDict === null) {
        paramDict = {};
      }
      const key = decodeURIComponent(paramStrings[0]);
      const value = decodeURIComponent(paramStrings[1]);
      paramDict[key] = value;
    }

    return {tabHash, parameters: paramDict};
  }

  return MainView;
})();

function ConstantsObserver() {}

/**
 * Loads all constants from |constants|.  On failure, global dictionaries are
 * not modifed.
 * @param {Object} receivedConstants The map of received constants.
 */
ConstantsObserver.prototype.onReceivedConstants = function(receivedConstants) {
  if (!areValidConstants(receivedConstants)) {
    return;
  }

  Constants = receivedConstants;

  EventType = Constants.logEventTypes;
  EventTypeNames = makeInverseMap(EventType);
  EventPhase = Constants.logEventPhase;
  EventSourceType = Constants.logSourceType;
  EventSourceTypeNames = makeInverseMap(EventSourceType);
  ClientInfo = Constants.clientInfo;
  LoadFlag = Constants.loadFlag;
  NetError = Constants.netError;
  QuicError = Constants.quicError;
  QuicRstStreamError = Constants.quicRstStreamError;
  AddressFamily = Constants.addressFamily;
  LoadState = Constants.loadState;
  DataReductionProxyBypassEventType =
      Constants.dataReductionProxyBypassEventType;
  DataReductionProxyBypassActionType =
      Constants.dataReductionProxyBypassActionType;
  // certStatusFlag may not be present when loading old log Files
  if (typeof(Constants.certStatusFlag) === 'object') {
    CertStatusFlag = Constants.certStatusFlag;
  } else {
    CertStatusFlag = {};
  }
  CertVerifierFlags = Constants.certVerifierFlags;
  CertVerifyFlags = Constants.certVerifyFlags;
  CertPathBuilderDigestPolicy = Constants.certPathBuilderDigestPolicy;

  timeutil.setTimeTickOffset(Constants.timeTickOffset);
};


function isNetLogNumber(value) {
  return (typeof(value) === 'number') ||
         (typeof(value) === 'string' && !isNaN(parseInt(value)));
}

/**
 * Returns true if it's given a valid-looking constants object.
 * @param {Object} receivedConstants The received map of constants.
 * @return {boolean} True if the |receivedConstants| object appears valid.
 */
function areValidConstants(receivedConstants) {
  return typeof(receivedConstants) === 'object' &&
      typeof(receivedConstants.logEventTypes) === 'object' &&
      typeof(receivedConstants.clientInfo) === 'object' &&
      typeof(receivedConstants.logEventPhase) === 'object' &&
      typeof(receivedConstants.logSourceType) === 'object' &&
      typeof(receivedConstants.loadFlag) === 'object' &&
      typeof(receivedConstants.netError) === 'object' &&
      typeof(receivedConstants.addressFamily) === 'object' &&
      isNetLogNumber(receivedConstants.timeTickOffset) &&
      typeof(receivedConstants.logFormatVersion) === 'number';
}

/**
 * Returns the name for netError.
 *
 * Example: netErrorToString(-105) should return
 * "ERR_NAME_NOT_RESOLVED".
 * @param {number} netError The net error code.
 * @return {string} The name of the given error.
 */
function netErrorToString(netError) {
  return getKeyWithValue(NetError, netError);
}

/**
 * Returns the name for quicError.
 *
 * Example: quicErrorToString(25) should return
 * "TIMED_OUT".
 * @param {number} quicError The QUIC error code.
 * @return {string} The name of the given error.
 */
function quicErrorToString(quicError) {
  return getKeyWithValue(QuicError, quicError);
}

/**
 * Returns the name for quicRstStreamError.
 *
 * Example: quicRstStreamErrorToString(3) should return
 * "BAD_APPLICATION_PAYLOAD".
 * @param {number} quicRstStreamError The QUIC RST_STREAM error code.
 * @return {string} The name of the given error.
 */
function quicRstStreamErrorToString(quicRstStreamError) {
  return getKeyWithValue(QuicRstStreamError, quicRstStreamError);
}

/**
 * Returns a string representation of |family|.
 * @param {number} family An AddressFamily
 * @return {string} A representation of the given family.
 */
function addressFamilyToString(family) {
  const str = getKeyWithValue(AddressFamily, family);
  // All the address family start with ADDRESS_FAMILY_*.
  // Strip that prefix since it is redundant and only clutters the output.
  return str.replace(/^ADDRESS_FAMILY_/, '');
}
