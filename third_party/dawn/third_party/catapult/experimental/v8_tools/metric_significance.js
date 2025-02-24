'use strict';
/**
 * Performs a test of significance on supplied metric data.
 */
class MetricSignificance {
  constructor() {
    /**  @private @const {Object} data_ The data to perform tests on. */
    this.data_ = {};
    /** @private @const {number} criticalPValue_
     * The default p value below which the null hypothesis will be rejected.
     */
    this.criticalPValue_ = 0.05;
    this.referenceColumn_ = undefined;
    this.IMPROVEMENT = 'improvement';
    this.REGRESSION = 'regression';
  }
  /**
   * Adds the given values to an entry for the supplied metric and
   * label. Tests of significance will be performed against the supplied
   * labels for the given metric. Therefore, each metric should be supplied
   * two labels.
   * @param {string} metric
   * @param {string} label
   * @param {Array<number>} value
   */
  add(metric, label, story, value) {
    if (this.referenceColumn_ === undefined) {
      this.referenceColumn_ = label;
    }
    if (!this.data_[metric]) {
      this.data_[metric] = {};
    }
    if (!this.data_[metric][label]) {
      this.data_[metric][label] = {};
    }
    if (!this.data_[metric][label][story]) {
      this.data_[metric][label][story] = [];
    }
    const values = this.data_[metric][label][story];
    this.data_[metric][label][story] = values.concat(value);
  }

  get referenceColumn() {
    return this.referenceColumn_;
  }

  set referenceColumn(referenceColumn) {
    this.referenceColumn_ = referenceColumn;
  }
  mostSignificantStories_(metricData, otherCol) {
    const significantChanges = [];
    const storyNames = Object.keys(metricData[this.referenceColumn_]);
    for (const story of storyNames) {
      const firstRun = metricData[this.referenceColumn_][story];
      const secondRun = metricData[otherCol][story];
      const evidence = this.test_(firstRun, secondRun);
      if (evidence.p < this.criticalPValue_) {
        significantChanges.push({
          story,
          evidence,
        });
      }
    }
    return significantChanges;
  }
  test_(xs, ys) {
    let evidence = mannwhitneyu.test(xs, ys, 'greater');
    // xs > ys, so ys uses less memory and it is an improvement.
    evidence.type = this.IMPROVEMENT;
    if (evidence.p >= this.criticalPValue_) {
      // There isn't evidence for a improvement in ys, so check for a
      // regressiom.
      evidence = mannwhitneyu.test(xs, ys, 'less');
      evidence.type = this.REGRESSION;
    }
    return evidence;
  }
  getAllData_(metricData, label) {
    const stories = Object.values(metricData[label]);
    // The data for a given metric and label is stored under each story for
    // that metric, so collect all of the data for each story.
    return stories.reduce(
        (collectedSoFar, storyData) => collectedSoFar.concat(storyData), []);
  }
  /**
   * Returns the metrics which have been identified as having statistically
   * significant changes along with the evidence supporting this (the p values
   * and U values).
   * @return {Array<Object>} Name and evidence of metrics with
   * statistically significant changes.
   */
  mostSignificant() {
    const significantChanges = [];
    Object.entries(this.data_).forEach(([metric, metricData]) => {
      const labels = Object.keys(metricData);
      const numLabels = labels.length;
      if (numLabels !== 2) {
        throw new Error(
            `Expected metric to have only two labels, received ${numLabels}`);
      }
      if (!labels.includes(this.referenceColumn_)) {
        throw new Error('Reference column not in labels');
      }
      const otherCol =
          labels[0] === this.referenceColumn_ ? labels[1] : labels[0];
      const xs = this.getAllData_(metricData, this.referenceColumn_);
      const ys = this.getAllData_(metricData, otherCol);
      const evidence = this.test_(xs, ys);
      const stories = this.mostSignificantStories_(metricData, otherCol);
      if (evidence.p < this.criticalPValue_) {
        significantChanges.push({
          metric,
          evidence,
          stories,
        });
      }
    });
    return significantChanges;
  }
}
