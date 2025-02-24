//  Vue component for drop-down menu; here the metrics,
//  stories and diagnostics are chosen through selection.
'use strict';
const app = new Vue({
  el: '#app',
  data: {
    state: {
      parsedMetrics: [],
      gridData: [],
      typesOfPlot: ['Cumulative frequency plot', 'Dot plot'],
      chosenTypeOfPlot: null,
      searchQuery: '',
      currentState: true,
    },
    sampleArr: [],
    guidValue: null,
    graph: new GraphData(),
    gridColumns: ['metric'],
    columnsForChosenDiagnostic: null,
    defaultGridData: [],
    stateManager: new StateManager(),
  },

  methods: {
    //  Reset the table content by returning to the
    //  previous default way with all the components
    //  available.
    resetTableData() {
      this.state.typesOfPlot = ['Cumulative frequency plot', 'Dot plot'];
      this.state.gridData = this.defaultGridData;
    },

    //  Get all stories for a specific metric.
    getStoriesByMetric(entry) {
      const stories = [];
      for (const e of this.sampleArr) {
        if (e.name !== entry) {
          continue;
        }
        let nameOfStory = this.guidValue.get(e.diagnostics.stories);
        if (nameOfStory === undefined) {
          continue;
        }
        if (typeof nameOfStory !== 'number') {
          nameOfStory = nameOfStory[0];
        }
        stories.push(nameOfStory);
      }
      return _.uniq(stories);
    },

    //  Extract a diagnostic from a specific
    //  element (like metric). This should be 'parsed' because
    //  sometimes it might be either a number or a
    //  single element array.
    getDiagnostic(elem, diagnostic) {
      let currentDiagnostic = this.guidValue.
          get(elem.diagnostics[diagnostic]);
      if (currentDiagnostic === undefined) {
        return undefined;
      }
      if (currentDiagnostic !== 'number') {
        currentDiagnostic = currentDiagnostic[0];
      }
      return currentDiagnostic;
    },

    //  This method creates an object for multiple metrics,
    //  multiple stories and some diagnostics:
    //  {labelName: {storyName: { metricName: sampleValuesArray}}}
    computeDataForStackPlot(metricsDependingOnGrid,
        storiesName, labelsName) {
      const obj = {};
      for (const elem of metricsDependingOnGrid) {
        const currentDiagnostic = this.getDiagnostic(elem, 'labels');
        if (currentDiagnostic === undefined) {
          continue;
        }
        const nameOfStory = this.getDiagnostic(elem, 'stories');
        if (nameOfStory === undefined) {
          continue;
        }

        if (storiesName.includes(nameOfStory) &&
          labelsName.includes(currentDiagnostic)) {
          let storyToMetricValues = {};
          if (obj.hasOwnProperty(currentDiagnostic)) {
            storyToMetricValues = obj[currentDiagnostic];
          }

          let metricToSampleValues = {};
          if (storyToMetricValues.hasOwnProperty(nameOfStory)) {
            metricToSampleValues = storyToMetricValues[nameOfStory];
          }

          let array = [];
          if (metricToSampleValues.hasOwnProperty(elem.name)) {
            array = metricToSampleValues[elem.name];
          }
          array = array.concat(average(elem.sampleValues));
          metricToSampleValues[elem.name] = array;
          storyToMetricValues[nameOfStory] = metricToSampleValues;
          obj[currentDiagnostic] = storyToMetricValues;
        }
      }
      return obj;
    },

    //  Draw a bar chart.
    plotBarChart(data) {
      this.pushCurrentState();
      this.graph.xAxis('Story')
          .yAxis('Memory used (MiB)')
          .title('Labels')
          .setData(data, story => app.$emit('bar_clicked', story))
          .plotBar();
    },

    //  Draw a dot plot depending on the target value.
    //  This is mainly for results from the table.
    plotDotPlot(target, story, traces) {
      this.pushCurrentState();
      const openTrace = (label, index) => {
        window.open(traces[label][index]);
      };
      this.graph
          .yAxis('')
          .xAxis('Memory used (MiB)')
          .title(story)
          .setData(target, openTrace)
          .plotDot();
    },

    //  Draw a cumulative frequency plot depending on the target value.
    //  This is mainly for the results from the table.
    plotCumulativeFrequencyPlot(target, story, traces) {
      const openTrace = (label, index) => {
        window.open(traces[label][index]);
      };
      this.pushCurrentState();
      this.graph.yAxis('Cumulative frequency')
          .xAxis('Memory used (MiB)')
          .title(story)
          .setData(target, openTrace)
          .plotCumulativeFrequency();
    },

    plotStackBar(obj, title) {
      this.pushCurrentState();
      this.graph.xAxis('Stories')
          .yAxis('Memory used (MiB)')
          .title(title)
          .setData(obj, metric => app.$emit('stack_clicked', metric))
          .plotStackedBar();
    },

    /**
     * Pushes the current state of data being displayed onto a stack to
     * be retrieved when an undo occurs.
     */
    pushCurrentState() {
      // Only states just popped of the stack have current state set to
      // false and should not be pushed back on.
      if (this.state.currentState) {
        // The state that is pushed is not current and should be marked
        // as such so that it is not pushed back onto the stack when an
        // undo occurs and the graph is plotted.
        this.state.currentState = false;
        // Deep clone the state objects so that their state is saved
        // and not mutated by future changes in Vue's state object.
        const clone = obj => JSON.parse(JSON.stringify(obj));
        this.stateManager.pushState({
          app: clone(this.state),
          menu: clone(menu.state),
          table: clone(this.$refs.tableComponent.state),
        });
      }
      this.state.currentState = true;
    },

    /**
     * Pops the previous state from the top of the stack and amends the
     * state objects of the vue components. This will trigger the
     * affected watchers and cause the graph to be plotted again based
     * on the retrieved data.
     */
    undo() {
      const savedState = this.stateManager.popState();
      this.replaceState(this.state, savedState.app);
      this.replaceState(menu.state, savedState.menu);
      this.replaceState(this.$refs.tableComponent.state, savedState.table);
    },

    /**
     * Checks if the state manager has any state saved onto the stack.
     */
    hasHistory() {
      return this.stateManager.hasHistory();
    },

    replaceState(oldState, newState) {
      Object.keys(oldState).forEach((key) => {
        // Only replace properties which have actually changed to
        // avoid triggering watchers for data which has not really
        // changed.
        if (!_.isEqual(oldState[key], newState[key])) {
          oldState[key] = newState[key];
        }
      });
    },
    //  Being given a metric, a story, a diagnostic and a set of
    //  subdiagnostics (i.e. 3 labels from the total of 4), the
    //  method return the sample values for each subdiagnostic.
    getSubdiagnostics(
        getTargetValueFromSample, metric, story, diagnostic, diagnostics) {
      const result = this.sampleArr
          .filter(value => value.name === metric &&
          this.guidValue
              .get(value.diagnostics.stories)[0] ===
              story);

      const content = new Map();
      for (const val of result) {
        const diagnosticItem = this.guidValue.get(
            val.diagnostics[diagnostic]);
        if (diagnosticItem === undefined) {
          continue;
        }
        let currentDiagnostic = '';
        if (typeof diagnosticItem === 'number') {
          currentDiagnostic = diagnosticItem;
        } else {
          currentDiagnostic = diagnosticItem[0];
        }
        const targetValue = getTargetValueFromSample(val);
        if (content.has(currentDiagnostic)) {
          const aux = content.get(currentDiagnostic);
          content.set(currentDiagnostic, aux.concat(targetValue));
        } else {
          content.set(currentDiagnostic, targetValue);
        }
      }
      const obj = {};
      for (const [key, value] of content.entries()) {
        if (diagnostics === undefined ||
          diagnostics.includes(key.toString())) {
          obj[key] = value;
        }
      }
      return obj;
    },

    getSampleValues(sample) {
      const toMiB = (x) => (x / MiB).toFixed(5);
      const values = sample.sampleValues;
      return values.map(value => toMiB(value));
    },

    getTraceLinks(sample) {
      const traceId = sample.diagnostics.traceUrls;
      return this.guidValue.get(traceId);
    },
    //  Draw a plot by default with all the sub-diagnostics
    //  in the same plot;
    plotSingleMetricWithAllSubdiagnostics(metric, story, diagnostic) {
      const obj = this.getSubdiagnostics(
          this.getSampleValues, metric, story, diagnostic);
      const traces = this.targetForMultipleDiagnostics(
          this.getTraceLinks, metric, story, diagnostic);
      this.plotCumulativeFrequencyPlot(obj, story, traces);
    },

    //  Draw a plot depending on the target value which is made
    //  of a metric, a story, a diagnostic and a couple of sub-diagnostics
    //  and the chosen type of plot. All are chosen from the table.
    plotSingleMetric(metric, story, diagnostic,
        diagnostics, chosenPlot) {
      this.state.chosenTypeOfPlot = chosenPlot;
      const target = this.targetForMultipleDiagnostics(
          this.getSampleValues, metric, story, diagnostic, diagnostics);
      const traces = this.targetForMultipleDiagnostics(
          this.getTraceLinks, metric, story, diagnostic, diagnostics);
      if (chosenPlot === 'Dot plot') {
        this.plotDotPlot(target, story, traces);
      } else {
        this.plotCumulativeFrequencyPlot(target, story, traces);
      }
    },

    //  Compute the target when the metric, story, diagnostics and
    //  sub-diagnostics are chosen from the table, not from the drop-down menu.
    //  It should be the same for both components but for now they should
    //  be divided.
    targetForMultipleDiagnostics(
        getTargetValueFromSample, metric, story, diagnostic, diagnostics) {
      if (metric === null || story === null ||
        diagnostic === null || diagnostics === null) {
        return undefined;
      }
      return this.getSubdiagnostics(
          getTargetValueFromSample, metric, story, diagnostic, diagnostics);
    }
  },

  computed: {
    gridDataLoaded() {
      return this.state.gridData.length > 0;
    },
    data_loaded() {
      return this.sampleArr.length > 0;
    }
  },

  watch: {

    //  Whenever we have new inputs from the menu (parsed inputs that
    //  where obtained by choosing from the tree) these should be
    //  added in the table (adding the average sample value).
    //  Also it creates by default a stack plot for all the metrics
    //  obtained from the tree-menu, all the stories from the top-level
    //  metric and all available labels.
    'state.parsedMetrics'() {
      if (this.state.parsedMetrics.length === 0) {
        // The menu has no metrics selected (the default state) so show
        // the entire table.
        this.resetTableData();
        return;
      }
      const newGridData = [];
      for (const metric of this.state.parsedMetrics) {
        for (const elem of this.defaultGridData) {
          if (elem.metric === metric) {
            newGridData.push(elem);
          }
        }
      }
      this.state.gridData = newGridData;

      //  We select from sampleValues all the metrics thath
      //  corespond to the result from tree menu (gridData)
      const metricsDependingOnGrid = [];
      const gridMetricsName = [];

      for (const metric of this.state.gridData) {
        gridMetricsName.push(metric.metric);
      }

      for (const metric of this.sampleArr) {
        if (gridMetricsName.includes(metric.name)) {
          metricsDependingOnGrid.push(metric);
        }
      }
      //  The top level metric is taken as source in
      //  computing stories.
      const storiesName =
          this.getStoriesByMetric(this.state.gridData[0].metric);
      const labelsName = this.columnsForChosenDiagnostic;
      const obj = this.computeDataForStackPlot(metricsDependingOnGrid,
          storiesName, labelsName);
      this.plotStackBar(obj, newGridData[0].metric);
      //  From now on the user will be able to switch between
      //  this 2 types of plot (taking into consideration that
      //  the scope of the tree-menu is to analyse using the
      //  the stacked plot and bar plot, we avoid for the moment
      //  other types of plot that should be actually used without
      //  using the tree menu)
      this.state.typesOfPlot = ['Bar chart plot', 'Stacked bar plot',
        'Cumulative frequency plot', 'Dot plot'];
      this.state.chosenTypeOfPlot = 'Stacked bar plot';
    }
  }
});
