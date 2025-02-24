var DEFAULT_RESOLUTION = 5;
var OUTLIER_THRESHOLD = 0.10;
var MARGIN_SIZE = 0.10;

Heatmap = function(canvas, resolutionSlider, data, colors) {
  this.canvas = canvas.get(0);
  this.context = this.canvas.getContext('2d');
  this.scaleCanvas();

  this.colors = colors;
  this.resolution = this.h / DEFAULT_RESOLUTION;

  this.calculate(data);
  this.installInputHandlers(canvas, resolutionSlider);

  this.draw();
};

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

  for (var trace = 0; trace < data.length; ++trace) {
    for (var t = 0; t < data[trace].data.length; ++t) {
      var revision = data[trace].data[t][0];
      var value = data[trace].data[t][1];
      if (revision > 1000000)
        continue;
      if (value == null)
        continue;

      if (!this.traces[revision])
        this.traces[revision] = {};
      this.traces[revision][trace] = value;
    }
  }

  this.revisions = Object.keys(this.traces).sort();
};

Heatmap.prototype.calculateBounds = function() {
  var values = [];
  for (var revision in this.traces)
    for (var trace in this.traces[revision])
      values.push(this.traces[revision][trace]);

  // Exclude OUTLIER_THRESHOLD% of the points.
  values.sort(function(a, b) {return a - b});
  this.min = percentile(values, OUTLIER_THRESHOLD / 2);
  this.max = percentile(values, -OUTLIER_THRESHOLD / 2);

  // Ease bounds by adding margins.
  var margin = (this.max - this.min) * MARGIN_SIZE * 2;
  this.min -= margin;
  this.max += margin;
};

Heatmap.prototype.calculateHeatmap = function() {
  this.data = {};
  for (var revision in this.traces) {
    for (var trace in this.traces[revision]) {
      var value = this.traces[revision][trace];
      y = Math.floor(mapRange(value, this.min, this.max, 0, this.resolution));
      y = constrain(y, 0, this.resolution - 1);

      if (this.data[revision] == null)
        this.data[revision] = {};
      if (this.data[revision][y] == null)
        this.data[revision][y] = [];
      this.data[revision][y].push(trace);
    }
  }
};

Heatmap.prototype.draw = function() {
  this.drawHeatmap();
  for (var i = 0; i < this.drawTraces.length; ++i)
    if (this.drawTraces[i])
      this.drawTrace(i, 1);
};

Heatmap.prototype.drawHeatmap = function() {
  this.context.clearRect(0, 0, this.w, this.h);

  this.context.save();
  this.context.scale(this.w / this.revisions.length, this.h / this.resolution);

  var counts = [];
  for (var t = 0; t < this.revisions.length; ++t) {
    var revision = this.revisions[t];
    for (var y in this.data[revision]) {
      counts.push(this.data[revision][y].length);
    }
  }
  counts.sort(function(a, b) {return a - b});
  var cutoff = percentile(counts, 0.9);
  if (cutoff < 2)
    cutoff = 2;

  for (var t = 0; t < this.revisions.length; ++t) {
    var revision = this.revisions[t];
    for (var y in this.data[revision]) {
      var count = this.data[revision][y].length;

      // Calculate average color across all traces in bucket.
      var r = 0, g = 0, b = 0;
      for (var i = 0; i < this.data[revision][y].length; ++i) {
        var trace = this.data[revision][y][i];
        r += this.colors[trace][0];
        g += this.colors[trace][1];
        b += this.colors[trace][2];
      }
      r /= count, g /= count, b /= count;
      var brightness = mapRange(count / cutoff, 0, 1, 2, 0);

      // Draw!
      this.context.fillStyle = calculateColor(r, g, b, 1, brightness);
      this.context.fillRect(t, y, 1, 1);
    }
  }

  this.context.restore();
};

Heatmap.prototype.drawTrace = function(trace, opacity) {
  this.drawTraceLine(trace, 4, 'rgba(255, 255, 255, ' + opacity + ')');
  var color = 'rgba(' + this.colors[trace][0] + ',' + this.colors[trace][1] + ',' + this.colors[trace][2] + ',' + opacity + ')';
  this.drawTraceLine(trace, 2, color);
};

Heatmap.prototype.drawTraceLine = function(trace, width, color) {
  var revisionWidth = this.w / this.revisions.length;

  this.context.save();
  this.context.lineJoin = 'round';
  this.context.lineWidth = width;
  this.context.strokeStyle = color;
  this.context.translate(revisionWidth / 2, 0);
  this.context.beginPath();

  var started = false;
  for (var t = 0; t < this.revisions.length; ++t) {
    var value = this.traces[this.revisions[t]][trace];
    if (value == null)
      continue;
    var y = mapRange(value, this.min, this.max, 0, this.h);
    if (started) {
      this.context.lineTo(revisionWidth * t, y);
    } else {
      this.context.moveTo(revisionWidth * t, y);
      started = true;
    }
  }

  this.context.stroke();
  this.context.restore();
}

Heatmap.prototype.scaleCanvas = function() {
  this.canvas.width = this.canvas.clientWidth * window.devicePixelRatio;
  this.canvas.height = this.canvas.clientHeight * window.devicePixelRatio;
  this.context.scale(window.devicePixelRatio, window.devicePixelRatio);

  this.w = this.canvas.clientWidth, this.h = this.canvas.clientHeight;

  // Flip canvas.
  this.context.scale(1, -1);
  this.context.translate(0, -this.h);
};
