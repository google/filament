'use strict';
//  Register the table component for displaying data.
//  This is a child for the app Vue instance, so it might
//  access some of the app's fields.
Vue.component('data-table', {
  template: '#table-template',
  props: {
    data: Array,
    columns: Array,
    filterKey: String,
    additional: Array,
    plot: String
  },
  mounted() {
    // TODO(anthonyalridge): Should also update table state.
    const jumpToStory = (story) => {
      this.plot = 'Cumulative frequency plot';
      const diagnostic = 'labels';
      app.plotSingleMetric(
          this.state.metric.metric,
          story,
          diagnostic,
          this.state.markedTableDiagnostics,
          this.plot);
    };
    app.$on('bar_clicked', jumpToStory);
    app.pushCurrentState();
  },
  data() {
    const sort = {};
    this.columns.forEach(function(key) {
      sort[key] = 1;
    });
    return {
      state: {
        sortKey: '',
        sortOrders: sort,
        openedMetric: [],
        openedStory: [],
        storiesEntries: null,
        metric: null,
        story: null,
        diagnostic: 'storysetRepeats',
        selected_diagnostics: [],
        markedTableMetrics: [],
        markedTableStories: [],
        markedTableDiagnostics: [],
      },
    };
  },
  computed: {
    //  Filter data from one column.
    filteredData() {
      const sortKey = this.state.sortKey;
      const filterKey = this.filterKey && this.filterKey.toLowerCase();
      const order = this.state.sortOrders[sortKey] || 1;
      let data = this.data;
      if (filterKey) {
        data = data.filter(function(row) {
          return Object.keys(row).some(function(key) {
            return String(row[key]).toLowerCase().indexOf(filterKey) > -1;
          });
        });
      }
      if (sortKey) {
        data = data.slice().sort(function(a, b) {
          a = a[sortKey];
          b = b[sortKey];
          return (a === b ? 0 : a > b ? 1 : -1) * order;
        });
      }
      return data;
    },

    //  All sub-diagnostics must be visible just after the user
    //  has already chosen a specific diagnostic and all the
    //  options for that one are now available.
    seen_diagnostics() {
      return this.diagnostics_options.length > 0 ? true : false;
    },

    //  Compute all the options for sub-diagnostics after the user
    //  has already chosen a specific diagnostic.
    //  Depending on the GUID of that diagnostic, the value can be
    //  a string, a number or undefined.
    diagnostics_options() {
      if (this.state.story !== null &&
      this.state.metric !== null &&
      this.state.diagnostic !== null) {
        const sampleArr = this.$parent.sampleArr;
        const guidValue = this.$parent.guidValue;
        const result = sampleArr
            .filter(value => value.name === this.state.metric.metric &&
                  guidValue
                      .get(value.diagnostics.stories)[0] ===
                      this.state.story.story);
        const content = [];
        for (const val of result) {
          const diagnosticItem = guidValue.get(
              val.diagnostics[this.state.diagnostic]);
          if (diagnosticItem === undefined) {
            continue;
          }
          let currentDiagnostic = '';
          if (typeof diagnosticItem === 'number') {
            currentDiagnostic = diagnosticItem.toString();
          } else {
            currentDiagnostic = diagnosticItem[0];
            if (typeof currentDiagnostic === 'number') {
              currentDiagnostic = currentDiagnostic.toString();
            }
          }
          content.push(currentDiagnostic);
        }
        return _.uniq(content);
      }
      return undefined;
    }
  },

  //  Capitalize the objects field names.
  filters: {
    capitalize(str) {
      if (typeof str === 'number') {
        return str.toString();
      }
      return str.charAt(0).toUpperCase() + str.slice(1);
    }
  },

  methods: {

    uncheckLabelsButtons() {
      const checkboxes = document.getElementsByClassName('checkbox-head');
      Array.prototype.map.call(checkboxes, function(checkbox) {
        checkbox.checked = false;
      });
      this.markedTableDiagnostics = [];
    },
    //  Sort by key where the key is a title head in table.
    sortBy(key) {
      this.state.sortKey = key;
      this.state.sortOrders[key] = this.state.sortOrders[key] * -1;
    },

    //  Remove all the selected items from the array.
    //  Especially for the cases when the user changes the mind and select
    //  another high level diagnostic and the selected sub-diagnostics
    //  will not be usefull anymore.
    empty() {
      this.state.selected_diagnostics = [];
    },

    //  Compute all the sample values depending on a single
    //  metric for each stories and for multiple sub-diagnostics.
    getSampleByStoryBySubdiagnostics(entry, sampleArr, guidValue, globalDiag) {
      const diagValues = new Map();
      for (const e of sampleArr) {
        if (e.name !== entry.metric) {
          continue;
        }
        let nameOfStory = guidValue.get(e.diagnostics.stories);
        if (nameOfStory === undefined) {
          continue;
        }
        if (typeof nameOfStory !== 'number') {
          nameOfStory = nameOfStory[0];
        }
        let diagnostic = guidValue.
            get(e.diagnostics[globalDiag]);
        if (diagnostic === undefined) {
          continue;
        }
        if (diagnostic !== 'number') {
          diagnostic = diagnostic[0];
        }
        if (!diagValues.has(nameOfStory)) {
          const map = new Map();
          map.set(diagnostic, [average(e.sampleValues)]);
          diagValues.set(nameOfStory, map);
        } else {
          const map = diagValues.get(nameOfStory);
          if (!map.has(diagnostic)) {
            map.set(diagnostic, [average(e.sampleValues)]);
            diagValues.set(nameOfStory, map);
          } else {
            const array = map.get(diagnostic);
            array.push(average(e.sampleValues));
            map.set(diagnostic, array);
            diagValues.set(nameOfStory, map);
          }
        }
      }
      return diagValues;
    },

    getStoriesByMetric(entry, sampleArr, guidValue) {
      const stories = [];
      for (const e of sampleArr) {
        if (e.name !== entry) {
          continue;
        }
        let nameOfStory = guidValue.get(e.diagnostics.stories);
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

    //  This method will be called when the user clicks a specific
    //  'row in table' = 'metric' and we have to provide the stories for that.
    //  Also all the previous choices must be removed.
    toggleMetric(entry) {
      const index = this.state.openedMetric.indexOf(entry.id);
      if (index > -1) {
        this.state.openedMetric.splice(index, 1);
      } else {
        this.state.openedMetric.push(entry.id);
      }
      const sampleArr = this.$parent.sampleArr;
      const guidValue = this.$parent.guidValue;
      const globalDiag = this.$parent.globalDiagnostic;
      const addCol = this.$parent.columnsForChosenDiagnostic;
      const storiesEntries = [];


      const stories = this
          .getStoriesByMetric(entry.metric, sampleArr, guidValue);
      for (const key of stories) {
        storiesEntries.push({
          story: key
        });
      }
      if (addCol !== null) {
        const diagValues = this
            .getSampleByStoryBySubdiagnostics(entry,
                sampleArr, guidValue, 'labels');
        for (const e of storiesEntries) {
          for (const diag of addCol) {
            e[diag] = fromBytesToMiB(average(diagValues
                .get(e.story).get(diag)));
          }
        }
      }
      this.uncheckLabelsButtons();
      this.state.storiesEntries = storiesEntries;
      this.state.metric = entry;
      this.empty();
    },

    //  This method will be called when the user clicks a specific
    //  story row and we have to compute all the available diagnostics.
    //  Also all the previous choices regarding a diagnostic must be removed.
    toggleStory(story) {
      const index = this.state.openedStory.indexOf(story.story);
      if (index > -1) {
        this.state.openedStory.splice(index, 1);
      } else {
        this.state.openedStory.push(story.story);
      }
      const sampleArr = this.$parent.sampleArr;
      const guidValue = this.$parent.guidValue;
      const result = sampleArr
          .filter(value => value.name === this.state.metric.metric &&
              guidValue
                  .get(value.diagnostics.stories)[0] ===
                  story.story);
      const allDiagnostic = [];
      result.map(val => allDiagnostic.push(Object.keys(val.diagnostics)));
      this.state.story = story;
      app.plotSingleMetricWithAllSubdiagnostics(this.state.metric.metric,
          this.state.story.story, this.state.diagnostic);
      this.empty();
    },

    createPlot() {
      let diagnostic = this.state.diagnostic;
      let diagnostics = [];
      if (this.state.markedTableDiagnostics.length !== 0) {
        diagnostic = 'labels';
        diagnostics = this.state.markedTableDiagnostics;
      } else
      if (this.state.selected_diagnostics.length !== 0) {
        diagnostics = this.state.selected_diagnostics;
      } else {
        diagnostics = this.diagnostics_options;
      }
      app.plotSingleMetric(
          this.state.metric.metric,
          this.state.story.story,
          diagnostic,
          diagnostics,
          this.plot);
    },
    //  When the user pick a new metric for further analysis
    //  this one has to be stored. If this is already stored
    //  this means that the action is the reverse one: unpick.
    pickTableMetric(entry) {
      if (this.state.markedTableMetrics.includes(entry.metric)) {
        this.state.markedTableMetrics.splice(
            this.state.markedTableMetrics.indexOf(entry.metric), 1);
      } else {
        this.state.markedTableMetrics.push(entry.metric);
      }
    },

    //  Whenever the user pick a new metric for further analysis
    //  this one has to be stored. If it is already stored,
    //  this means that the user actually unpicked it.
    pickTableStory(entry) {
      if (this.state.markedTableStories.includes(entry.story)) {
        this.state.markedTableStories.splice(
            this.state.markedTableStories.indexOf(entry.story), 1);
      } else {
        this.state.markedTableStories.push(entry.story);
      }
    },

    //  The same for pickTableMetric and pickTableStory.
    pickHeadTable(title) {
      if (this.state.markedTableDiagnostics.includes(title)) {
        this.state.markedTableDiagnostics.splice(
            this.state.markedTableDiagnostics.indexOf(title), 1);
      } else {
        this.state.markedTableDiagnostics.push(title);
      }
    },

    //  Draw a bar chart when multiple stories are selected
    //  from a single metric and multiple sub-diagnostics are
    //  selected froma a single main diagnostic.
    plotMultipleStoriesMultipleDiag() {
      if (this.state.markedTableDiagnostics.length !== 0) {
        const sampleArr = this.$parent.sampleArr;
        const guidValue = this.$parent.guidValue;
        const map = this
            .getSampleByStoryBySubdiagnostics(this.state.metric,
                sampleArr, guidValue, 'labels');
        const data = {};
        for (const e of this.state.markedTableDiagnostics) {
          const obj = {};
          for (const story of this.state.markedTableStories) {
            obj[story] = map.get(story).get(e);
          }
          data[e] = obj;
        }
        app.plotBarChart(data);
      }
    },

    //  When the user selects a specific row from the table
    //  this does not mean that it is the only one metric
    //  with that name, so we have to extract all available
    //  metrics from sampleValues.
    getAllMetricsFromMetricRow() {
      const sampleArr = this.$parent.sampleArr;
      const markedMetrics = [];
      for (const metric of sampleArr) {
        for (const e of this.state.markedTableMetrics) {
          if (metric.name === e) {
            markedMetrics.push(metric);
          }
        }
      }
      return markedMetrics;
    },

    //  The metrics from grid are the ones that come
    //  after selecting items from tree-menu.
    //  We need to filter just that metrics from the total
    //  sampleValues metrics.
    getMetricsFromGrid() {
      const sampleArr = this.$parent.sampleArr;
      const gridData = this.$parent.state.gridData;
      const metricsDependingOnGrid = [];
      for (const metric of sampleArr) {
        for (const e of gridData) {
          if (metric.name === e.metric) {
            metricsDependingOnGrid.push(metric);
          }
        }
      }
      return metricsDependingOnGrid;
    }
  },

  watch: {
    //  Whenever a new diagnostic is chosen or removed, the graph
    //  is replotted because these are displayed in the same plot
    //  by comparison and it should be updated.
    'state.selected_diagnostics'() {
      if (this.state.selected_diagnostics.length !== 0) {
        this.uncheckLabelsButtons();
        this.createPlot();
      }
    },

    //  Whenever the chosen plot is changed by the user it has to
    //  be created another type of plot with the same specifications.
    plot() {
      if (this.plot === 'Cumulative frequency plot' ||
        this.plot === 'Dot plot') {
        this.createPlot();
      }
    },

    //  Whenever the top level diagnostic is changed all the previous
    //  selected sub-diagnostics have to be removed. Otherwise the old
    //  selections will be displayed. Also the plot is displayed with
    //  values for all available sub-diagnostics.
    'state.diagnostic'() {
      this.empty();
      app.plotSingleMetricWithAllSubdiagnostics(this.state.metric.metric,
          this.state.story.story, this.state.diagnostic);
    },

    //  Whenever a new subdiagnostic from table columns is chosen
    //  it is added to the chart. Depending on the main diagnostic
    //  and its subdiagnostics, all the sample values for a particular
    //  metric, multiple stories, a single main diagnostic and multiple
    //  subdiagnostics are computed. The plot is drawn using this data.
    'state.markedTableDiagnostics'() {
      if (this.state.markedTableDiagnostics.length !== 0) {
        const sampleArr = this.$parent.sampleArr;
        const guidValue = this.$parent.guidValue;
        if (this.state.markedTableMetrics.length === 0) {
          if (this.$parent.state.chosenTypeOfPlot === 'Stacked bar plot') {
            const markedMetrics = this.getMetricsFromGrid();
            const stories = this.getStoriesByMetric(
                app.state.gridData[0].metric, sampleArr, guidValue);
            const obj = app.computeDataForStackPlot(markedMetrics,
                stories, this.state.markedTableDiagnostics);
            const string = 'Stacked plot';
            app.plotStackBar(obj, string);
          } else if (this.$parent.chosenTypeOfPlot === 'Bar chart plot') {
            this.plotMultipleStoriesMultipleDiag();
          } else {
            this.createPlot();
            this.selected_diagnostics = [];
          }
        } else {
          const markedMetrics = this.getAllMetricsFromMetricRow();
          let stories = [];
          if (this.markedTableStories.length === 0) {
            stories = this.getStoriesByMetric(app
                .gridData[0].metric, sampleArr, guidValue);
          } else {
            stories = this.markedTableStories;
          }
          const obj = app.computeDataForStackPlot(markedMetrics,
              stories, this.state.markedTableDiagnostics);
          const string = 'Stacked plot';
          app.plotStackBar(obj, string);
        }
      }
    },

    //  Whenever a new story from table is chosen it has to be added
    //  in the final chart. The chart that should be updated might be
    //  a stacked chart or a bar chart in this particular case.
    'state.markedTableStories'() {
      if (this.state.markedTableMetrics.length === 0) {
        //  In this case the user wants to change the stories for
        //  the initial stacked plot obtained using all the metrics
        //  from grid, all the stories from top level metric and
        //  all the available options.
        if (this.$parent.state.chosenTypeOfPlot === 'Stacked bar plot') {
          const markedMetrics = this.getMetricsFromGrid();
          const labelsName = this.$parent.columnsForChosenDiagnostic;
          const obj = app.computeDataForStackPlot(markedMetrics,
              this.state.markedTableStories, labelsName);
          const string = 'Stacked plot';
          app.plotStackBar(obj, string);
        } else {
          this.plotMultipleStoriesMultipleDiag();
        }
      } else {
        //  The user wants to change the stories after having
        //  some selected metrics.
        const markedMetrics = this.getAllMetricsFromMetricRow();
        const labelsName = this.$parent.columnsForChosenDiagnostic;
        const obj = app.computeDataForStackPlot(markedMetrics,
            this.state.markedTableStories, labelsName);
        const string = 'Stacked plot';
        app.plotStackBar(obj, string);
      }
    },

    //  Whenever the main selected metric from the table is changed
    //  all marked diagnostics have to be removed because they are
    //  not available.
    'state.metric'() {
      this.state.markedTableDiagnostics = [];
    },

    //  Whenever a new metric is selected the stacked chart should
    //  be updated.
    'state.markedTableMetrics'() {
      const sampleArr = this.$parent.sampleArr;
      const guidValue = this.$parent.guidValue;
      //  As sources for final objet:
      //  1) the metrics are taken from sampleValues; these
      //  should have the same same as the selected row;
      const markedMetrics = this.getAllMetricsFromMetricRow();
      //  2) the stories are the ones from the top level metric;
      const stories = this.getStoriesByMetric(
          app.state.gridData[0].metric, sampleArr, guidValue);
      //  3) the labels are all the available labels;
      let labelsName = [];
      if (this.state.markedTableDiagnostics.length !== 0) {
        labelsName = this.state.markedTableDiagnostics;
      } else {
        labelsName = this.$parent.columnsForChosenDiagnostic;
      }
      const obj = app.computeDataForStackPlot(markedMetrics,
          stories, labelsName);
      const string = 'Stacked plot';
      app.plotStackBar(obj, string);
    }
  }
});
