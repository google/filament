'use strict';
Vue.component('stat-test-summary', {
  props: {
    testResults: Array,
    referenceColumn: String,
  },
  methods: {
    splitMemoryMetric(metric) {
      menu.splitMemoryMetric(metric);
    },
  },
  template: '#stat-test-summary-template',
});
