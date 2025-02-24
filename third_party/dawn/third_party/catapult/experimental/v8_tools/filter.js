'use strict';
const MiB = 1024 * 1024;
Vue.component('v-select', VueSelect.VueSelect);

const menu = new Vue({
  el: '#menu',
  data: {
    state: {
      browser: null,
      subprocess: null,
      component: null,
      size: null,
      subcomponent: null,
      subsubcomponent: null,
      testResults: [],
      referenceColumn: '',
    },
    browserOptions: [],
    subprocessOptions: [],
    metricNames: null,
    sampleArr: null,
    guidValueInfo: null,
    componentMap: null,
    sizeMap: null,

    allLabels: [],
    significanceTester: new MetricSignificance(),
  },
  mounted() {
    app.$on('stack_clicked', this.splitMemoryMetric);
  },
  computed: {
    //  Compute size options. The user will be provided with all
    //  sizes and the probe will be auto detected from it.
    sizeOptions() {
      if (this.sizeMap === null) {
        return undefined;
      }
      let sizes = [];
      for (const [key, value] of this.sizeMap.entries()) {
        sizes = sizes.concat(value);
      }
      return sizes;
    },

    //  The components are different depending on the type of probe.
    //  The probe is auto detected depending on the chosen size.
    //  Then the user is provided with the first level of components.
    componentsOptions() {
      if (this.componentMap === null || this.state.size === null) {
        return undefined;
      }
      for (const [key, value] of this.sizeMap.entries()) {
        if (value.includes(this.state.size)) {
          this.probe = key;
        }
      }
      const components = [];
      for (const [key, value] of this.componentMap.get(this.probe).entries()) {
        components.push(key);
      }
      return components;
    },

    //  Compute the options for the first subcomponent depending on the probes.
    //  When the user chooses a component, it might be a hierarchical one.
    firstSubcompOptions() {
      if (this.state.component === null) {
        return undefined;
      }
      const subcomponent = [];
      for (const [key, value] of this
          .componentMap.get(this.probe).get(this.state.component).entries()) {
        subcomponent.push(key);
      }
      return subcomponent;
    },

    //  In case when the component is from Chrome, the hierarchy might have more
    //  levels.
    secondSubcompOptions() {
      if (this.state.subcomponent === null) {
        return undefined;
      }
      const subcomponent = [];
      for (const [key, value] of this
          .componentMap
          .get(this.probe)
          .get(this.state.component)
          .get(this.state.subcomponent).entries()) {
        subcomponent.push(key);
      }
      return subcomponent;
    }
  },
  watch: {
    'state.size'() {
      this.state.component = null;
      this.state.subcomponent = null;
      this.state.subsubcomponent = null;
    },

    'state.component'() {
      this.state.subcomponent = null;
      this.state.subsubcomponent = null;
    },

    'state.subcomponent'() {
      this.state.subsubcomponent = null;
    },

    'state.referenceColumn'() {
      if (this.state.referenceColumn === null) {
        this.state.testResults = [];
        return;
      }
      this.significanceTester.referenceColumn = this.state.referenceColumn;
      this.state.testResults = this.significanceTester.mostSignificant();
    }

  },
  methods: {
    //  Build the available metrics upon the chosen items.
    //  The method applies an intersection for all of them and
    //  return the result as a collection of metrics that matched.
    //  Also the metric that exactly matches the menu selected items
    //  will be the first one in array and the first row in table.
    apply() {
      let nameOfMetric = 'memory:' +
        this.state.browser + ':' +
        this.state.subprocess + ':' +
        this.probe + ':' +
        this.state.component;
      if (this.state.subcomponent !== null) {
        nameOfMetric += ':' + this.state.subcomponent;
      }
      if (this.state.subsubcomponent !== null) {
        nameOfMetric += ':' + this.state.subsubcomponent;
      }
      let metrics = [];
      for (const name of this.metricNames) {
        if (this.state.browser !== null &&
          name.includes(this.state.browser) &&
          this.state.subprocess !== null &&
          name.includes(this.state.subprocess) &&
          this.state.component !== null &&
          name.includes(this.state.component) &&
          this.state.size !== null &&
          name.includes(this.state.size) &&
          this.probe !== null &&
          name.includes(this.probe)) {
          if (this.state.subcomponent === null) {
            metrics.push(name);
          } else {
            if (name.includes(this.state.subcomponent)) {
              if (this.state.subsubcomponent === null) {
                metrics.push(name);
              } else {
                if (name.includes(this.state.subsubcomponent)) {
                  metrics.push(name);
                }
              }
            }
          }
        }
      }
      nameOfMetric += ':' + this.state.size;
      if (_.uniq(metrics).length === 0) {
        alert('No metrics found');
      } else {
        metrics = _.uniq(metrics);
        metrics.splice(metrics.indexOf(nameOfMetric), 1);
        metrics.unshift(nameOfMetric);
        app.state.parsedMetrics = metrics;
      }
    },

    /**
     * Splits a memory metric into it's heirarchical data and
     * assigns this heirarchy information into the relavent fields
     * of the menu. It also updates the table to display only the
     * given metric.
     * @param {string} metricName The name of the metric to be split.
     */
    async splitMemoryMetric(metricName) {
      if (!metricName.startsWith('memory')) {
        throw new Error('Expected a memory metric');
      }
      const parts = metricName.split(':');
      const heirarchyInformation = {
        browser: 1,
        process: 2,
        probe: 3,
        componentStart: 4,
        // Metrics have a variable number of subcomponents
        // (.e.g, a metric which is an aggregate over subcomponents will
        // have one less subcomponent field than it's sub-metrics).
        // Therefore, the end of the subcomponents field and
        // location of the size field must be calculated dynamically.
        componentsEnd: parts.length - 2,
        size: parts.length - 1,
      };
      // Assigning to these fields updates the corresponding select
      // menu in the UI.
      this.state.browser = parts[heirarchyInformation.browser];
      this.state.subprocess = parts[heirarchyInformation.process];
      this.probe = parts[heirarchyInformation.probe];
      this.state.size = parts[heirarchyInformation.size];
      // The size watcher sets 'this.state.component' to null so we must wait
      // for the DOM to be updated. Then the size watcher is called before
      // assigning to 'this.state.component' and so it is not overwritten
      // with null.
      await this.$nextTick();
      this.state.component = parts[heirarchyInformation.componentStart];
      const start = heirarchyInformation.componentStart;
      const end = heirarchyInformation.componentsEnd;
      for (let i = start + 1; i <= end; i++) {
        const subcomponent = i - heirarchyInformation.componentStart;
        switch (subcomponent) {
          case 1: {
            // See above comment (component watcher sets subcomponent to null).
            await this.$nextTick();
            this.state.subcomponent = parts[i];
            break;
          }
          case 2: {
            // See above comment
            // (subcomponent watcher sets subsubcomponent to null).
            await this.$nextTick();
            this.state.subsubcomponent = parts[i];
            break;
          }
          default: throw new Error('Unexpected number of subcomponents.');
        }
      }
      app.state.parsedMetrics = [metricName];
    },
  }
});

function average(arr) {
  return _.reduce(arr, function(memo, num) {
    return memo + num;
  }, 0) / arr.length;
}

//  This function returns an object containing:
//  all the names of labels plus a map like this:
//  map: [metric_name] -> [map: [lable] -> sampleValue],
//  where the lable is each name of all sub-labels
//  and sampleValue is the average for a specific
//  metric across stories with a specific label.
function getMetricStoriesLabelsToValuesMap(sampleArr, guidValueInfo) {
  const newDiagnostics = new Set();
  const metricToDiagnosticValuesMap = new Map();
  for (const elem of sampleArr) {
    let currentDiagnostic = guidValueInfo.
        get(elem.diagnostics.labels);
    if (currentDiagnostic === undefined) {
      continue;
    }
    if (currentDiagnostic !== 'number') {
      currentDiagnostic = currentDiagnostic[0];
    }
    newDiagnostics.add(currentDiagnostic);

    if (!metricToDiagnosticValuesMap.has(elem.name)) {
      const map = new Map();
      map.set(currentDiagnostic, [average(elem.sampleValues)]);
      metricToDiagnosticValuesMap.set(elem.name, map);
    } else {
      const map = metricToDiagnosticValuesMap.get(elem.name);
      if (map.has(currentDiagnostic)) {
        const array = map.get(currentDiagnostic);
        array.push(average(elem.sampleValues));
        map.set(currentDiagnostic, array);
        metricToDiagnosticValuesMap.set(elem.name, map);
      } else {
        map.set(currentDiagnostic, [average(elem.sampleValues)]);
        metricToDiagnosticValuesMap.set(elem.name, map);
      }
    }
  }
  return {
    labelNames: Array.from(newDiagnostics),
    mapLabelToValues: metricToDiagnosticValuesMap
  };
}

function fromBytesToMiB(value) {
  return (value / MiB).toFixed(5);
}


//   Load the content of the file and further display the data.
function readSingleFile(e) {
  const file = e.target.files[0];
  if (!file) {
    return;
  }
  //  Extract data from file and distribute it in some relevant structures:
  //  results for all guid-related( for now they are not
  //  divided in 3 parts depending on the type ) and
  //  all results with sample-value-related and
  //  map guid to value within the same structure
  const reader = new FileReader();
  reader.onload = function(e) {
    const contents = extractData(e.target.result);
    const sampleArr = contents.sampleValueArray;
    const guidValueInfo = contents.guidValueInfo;
    const metricAverage = new Map();
    const allLabels = new Set();
    for (const e of sampleArr) {
      // This version of the tool focuses on analysing memory
      // metrics, which contain a slightly different structure
      // to the non-memory metrics.
      if (e.name.startsWith('memory')) {
        const { name, sampleValues, diagnostics } = e;
        const { labels, stories } = diagnostics;
        const label = guidValueInfo.get(labels)[0];
        allLabels.add(label);
        const story = guidValueInfo.get(stories)[0];
        menu.significanceTester.add(name, label, story, sampleValues);
      }
    }
    menu.allLabels = Array.from(allLabels);
    let metricNames = [];
    sampleArr.map(e => metricNames.push(e.name));
    metricNames = _.uniq(metricNames);


    //  The content for the default table: with name
    //  of the metric, the average value of the sample values
    //  plus an id. The latest is used to expand the row.
    //  It may disappear later.
    const tableElems = [];
    let id = 1;
    for (const name of metricNames) {
      tableElems.push({
        id: id++,
        metric: name
      });
    }

    const labelsResult = getMetricStoriesLabelsToValuesMap(
        sampleArr, guidValueInfo);
    const columnsForChosenDiagnostic = labelsResult.labelNames;
    const metricToDiagnosticValuesMap = labelsResult.mapLabelToValues;
    for (const elem of tableElems) {
      if (metricToDiagnosticValuesMap.get(elem.metric) === undefined) {
        continue;
      }
      for (const diagnostic of columnsForChosenDiagnostic) {
        if (!metricToDiagnosticValuesMap.get(elem.metric).has(diagnostic)) {
          continue;
        }
        elem[diagnostic] = fromBytesToMiB(average(metricToDiagnosticValuesMap
            .get(elem.metric).get(diagnostic)));
      }
    }


    app.state.gridData = tableElems;
    app.defaultGridData = tableElems;
    app.sampleArr = sampleArr;
    app.guidValue = guidValueInfo;
    app.columnsForChosenDiagnostic = columnsForChosenDiagnostic;

    const result = parseAllMetrics(metricNames);
    menu.sampelArr = sampleArr;
    menu.guidValueInfo = guidValueInfo;

    menu.browserOptions = result.browsers;
    menu.subprocessOptions = result.subprocesses;
    menu.componentMap = result.components;
    menu.sizeMap = result.sizes;
    menu.metricNames = result.names;
  };
  reader.readAsText(file);
}

function extractData(contents) {
  /*
   *  Populate guidValue with guidValue objects containing
   *  guid and value from the same type of data.
   */
  const guidValueInfoMap = new Map();
  const result = [];
  const sampleValue = [];
  const dateRangeMap = new Map();
  const other = [];
  /*
   *  Extract every piece of data between <histogram-json> tags;
   *  all data is written between these tags
   */
  const reg = /<histogram-json>(.*?)<\/histogram-json>/g;
  let m = reg.exec(contents);
  while (m !== null) {
    result.push(m[1]);
    m = reg.exec(contents);
  }
  for (const element of result) {
    const e = JSON.parse(element);
    if (e.hasOwnProperty('sampleValues')) {
      const elem = {
        name: e.name,
        sampleValues: e.sampleValues,
        unit: e.unit,
        guid: e.guid,
        diagnostics: {}
      };
      if (e.diagnostics === undefined || e.diagnostics === null) {
        continue;
      }
      if (e.diagnostics.hasOwnProperty('traceUrls')) {
        elem.diagnostics.traceUrls = e.diagnostics.traceUrls;
      }
      if (e.diagnostics.hasOwnProperty('labels')) {
        elem.diagnostics.labels = e.diagnostics.labels;
      } else {
        elem.diagnostics.labels = e.diagnostics.benchmarkStart;
      }
      if (e.diagnostics.hasOwnProperty('stories')) {
        elem.diagnostics.stories = e.diagnostics.stories;
      }
      if (e.diagnostics.hasOwnProperty('storysetRepeats')) {
        elem.diagnostics.storysetRepeats = e.diagnostics.storysetRepeats;
      }
      sampleValue.push(elem);
    } else {
      if (e.type === 'GenericSet') {
        guidValueInfoMap.set(e.guid, e.values);
      } else if (e.type === 'DateRange') {
        guidValueInfoMap.set(e.guid, e.min);
      } else {
        other.push(e);
      }
    }
  }

  return {
    guidValueInfo: guidValueInfoMap,
    guidMinInfo: dateRangeMap,
    otherTypes: other,
    sampleValueArray: sampleValue
  };
}
document.getElementById('file-input')
    .addEventListener('change', readSingleFile, false);
