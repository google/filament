var OUTLIER_THRESHOLD = 0.05;
var MARGIN_SIZE = 0.10;

Heatmap.prototype.calculate = function(data) {
  this.cleanUpData(data);
  this.calculateBounds();
  this.calculateHeatmap();

  this.drawTraces = [];
  for (var i = 0; i < data.length; ++i)
    this.drawTraces.push(false);
};

Heatmap.prototype.cleanUpData = function(data) {
  // Data, indexed by revision and trace.
  this.traces = {};

  for (var bot in data) {
    var bot_data = data[bot];
    for (var run_index = 0; run_index < bot_data.length; ++run_index) {
      var run_data = bot_data[run_index];
      if (run_data == null)
        continue
      var stories_data = run_data['user_story_runs'];
      for (var story_index = 0; story_index < stories_data.length; ++story_index) {
        var story_data = stories_data[story_index];
        var story_name = story_data['user_story'];
        if (story_name == 'summary')
          continue

        values = story_data['values'];

        var index = bot_data.length - run_index - 1;
        if (!this.traces[index])
          this.traces[index] = {};
        this.traces[index][story_index] = values;
      }
    }
  }

  this.revisions = Object.keys(this.traces).sort();
};

Heatmap.prototype.calculateBounds = function() {
  var values = [];
  for (var revision in this.traces)
    for (var trace in this.traces[revision])
      for (var value of this.traces[revision][trace])
        values.push(value);

  // Exclude OUTLIER_THRESHOLD% of the points.
  values.sort(function(a, b) {return a - b});
  this.min = percentile(values, OUTLIER_THRESHOLD / 2);
  this.max = percentile(values, -OUTLIER_THRESHOLD / 2);

  // Ease bounds by adding margins.
  var margin = (this.max - this.min) * MARGIN_SIZE;
  this.min -= margin;
  this.max += margin;
};

Heatmap.prototype.calculateHeatmap = function() {
  this.data = {};
  for (var revision in this.traces) {
    for (var trace in this.traces[revision]) {
      for (var value of this.traces[revision][trace]) {
        var bucket = this.findBucket(value);

        if (this.data[revision] == null)
          this.data[revision] = {};
        if (this.data[revision][bucket] == null)
          this.data[revision][bucket] = [];
        this.data[revision][bucket].push(trace);
      }
    }
  }
};

Heatmap.prototype.findBucket = function(value) {
  var bucket = Math.floor(mapRange(value, this.min, this.max, 0, this.resolution));
  return constrain(bucket, 0, this.resolution - 1);
};
