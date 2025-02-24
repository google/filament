/* Copyright 2019 The Chromium Authors. All rights reserved.
   Use of this source code is governed by a BSD-style license that can be
   found in the LICENSE file.
*/
'use strict';

window.METRICS = new class extends window.chops.tsmon.TSMonClient {
  constructor() {
    super();

    this.signedIn = false;
    this.frontendVersion = 0;

    const metadata = {};
    const fields = new Map([
      ['fe_version', this.constructor.intField('fe_version')],
      ['signed_in', this.constructor.boolField('signed_in')],
    ]);

    this.pageLoad_ = this.cumulativeDistribution(
        'chromeperf/load/page',
        'page loadEventEnd - fetchStart',
        metadata, fields);

    this.chartLoadStart_ = undefined;
    this.chartLoad_ = this.cumulativeDistribution(
        'chromeperf/load/chart',
        'chart load latency',
        metadata, fields);

    this.alertsLoadStart_ = undefined;
    this.alertsLoad_ = this.cumulativeDistribution(
        'chromeperf/load/alerts',
        'alerts load latency',
        metadata, fields);

    this.menuLoadStart_ = undefined;
    this.menuLoad_ = this.cumulativeDistribution(
        'chromeperf/load/menu',
        'menu load latency',
        metadata, fields);

    this.chartActionStart_ = undefined;
    this.chartAction_ = this.cumulativeDistribution(
        'chromeperf/action/chart',
        'timeseries picker activity duration',
        metadata, fields);

    this.triageActionStart_ = undefined;
    this.triageAction_ = this.cumulativeDistribution(
        'chromeperf/action/triage',
        'alert triage latency',
        metadata, fields);
  }

  get fields_() {
    return new Map([
      ['fe_version', this.frontendVersion],
      ['signed_in', this.signedIn],
    ]);
  }

  onLoad() {
    const ms = performance.timing.loadEventEnd - performance.timing.fetchStart;
    this.pageLoad_.add(ms, this.fields_);
  }

  startLoadChart() {
    this.chartLoadStart_ = performance.now();
  }

  endLoadChart() {
    if (this.chartLoadStart_ === undefined) return;
    const ms = performance.now() - this.chartLoadStart_;
    this.chartLoad_.add(ms, this.fields_);
    this.chartLoadStart_ = undefined;
  }

  startLoadAlerts() {
    this.alertsLoadStart_ = performance.now();
  }

  endLoadAlerts() {
    if (this.alertsLoadStart_ === undefined) return;
    const ms = performance.now() - this.alertsLoadStart_;
    this.alertsLoad_.add(ms, this.fields_);
    this.alertsLoadStart_ = undefined;
  }

  startLoadMenu() {
    this.menuLoadStart_ = performance.now();
  }

  endLoadMenu() {
    if (this.menuLoadStart_ === undefined) return;
    const ms = performance.now() - this.menuLoadStart_;
    this.menuLoad_.add(ms, this.fields_);
    this.menuLoadStart_ = undefined;
  }

  startChartAction() {
    this.chartActionStart_ = performance.now();
  }

  endChartAction() {
    if (this.chartActionStart_ === undefined) return;
    const ms = performance.now() - this.chartActionStart_;
    this.chartAction_.add(ms, this.fields_);
    this.chartActionStart_ = undefined;
  }

  startTriage() {
    this.triageActionStart_ = performance.now();
  }

  endTriage() {
    if (this.triageActionStart_ === undefined) return;
    const ms = performance.now() - this.triageActionStart_;
    this.triageAction_.add(ms, this.fields_);
    this.triageActionStart_ = undefined;
  }
};

window.addEventListener('load', () => setTimeout(() => METRICS.onLoad(), 1000));
