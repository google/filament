var DEFAULT_RESOLUTION = 5;

Heatmap.prototype.installInputHandlers = function(canvas, resolutionSlider) {
  var self = this;

  canvas.mousemove(function(event) {
    var mouseX = event.pageX - canvas.position().left;
    var mouseY = event.pageY - canvas.position().top;
    self.draw();
    var traces = self.findTracesAt(mouseX, mouseY);
    for (var i = 0; i < traces.length; ++i)
      self.drawTrace(traces[i], 0.5);
  });

  canvas.click(function(event) {
    var mouseX = event.pageX - canvas.position().left;
    var mouseY = event.pageY - canvas.position().top;
    var traces = self.findTracesAt(mouseX, mouseY);
    for (var i = 0; i < traces.length; ++i)
      self.selectTrace(traces[i]);
    self.draw();
  });

  resolutionSlider.val(DEFAULT_RESOLUTION);
  resolutionSlider.change(function() {
    self.resolution = self.h / this.value;
    self.calculateHeatmap();
    self.draw();
  });
  resolutionSlider.change();
};

Heatmap.prototype.selectTrace = function(trace) {
  this.drawTraces[trace] = !this.drawTraces[trace];
};

Heatmap.prototype.getTime = function(x) {
  var time = Math.floor(mapRange(x, 0, this.w, 0, this.revisions.length));
  return constrain(time, 0, this.revisions.length - 1);
};

Heatmap.prototype.getBucket = function(y) {
  var bucket = Math.floor(mapRange(y, this.h, 0, 0, this.resolution));
  return constrain(bucket, 0, this.resolution - 1);
};

Heatmap.prototype.findTracesAt = function(x, y) {
  var minX = x - 5, minY = y + 5;
  var maxX = x + 5, maxY = y - 5;

  var minTime = this.getTime(minX), minY = this.getBucket(minY);
  var maxTime = this.getTime(maxX), maxY = this.getBucket(maxY);

  var traces = {};  // Use an object to avoid duplicates.
  for (var time = minTime; time <= maxTime; ++time) {
    for (var bucket = minY; bucket <= maxY; ++bucket) {
      var revision = this.revisions[time];
      if (!this.data[revision])
        continue;
      if (!this.data[revision][bucket])
        continue;

      for (var i = 0; i < this.data[revision][bucket].length; ++i)
        traces[this.data[revision][bucket][i]] = true;
    }
  }

  return Object.keys(traces);
};
