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
  for (var time = 0; time < this.revisions.length; ++time) {
    var revision = this.revisions[time];
    for (var bucket in this.data[revision]) {
      counts.push(this.data[revision][bucket].length);
    }
  }
  counts.sort(function(a, b) {return a - b});
  var cutoff = percentile(counts, 0.9);
  if (cutoff < 2)
    cutoff = 2;

  for (var time = 0; time < this.revisions.length; ++time) {
    var revision = this.revisions[time];
    for (var bucket in this.data[revision]) {
      var count = this.data[revision][bucket].length;

      // Calculate average color across all traces in bucket.
      var r = 0, g = 0, b = 0;
      for (var i = 0; i < this.data[revision][bucket].length; ++i) {
        var trace = this.data[revision][bucket][i];
        r += nthColor(trace)[0];
        g += nthColor(trace)[1];
        b += nthColor(trace)[2];
      }
      r /= count, g /= count, b /= count;
      var brightness = mapRange(count / cutoff, 0, 1, 2, 0.5);

      // Draw!
      this.context.fillStyle = calculateColor(r, g, b, 1, brightness);
      this.context.fillRect(time, bucket, 1, 1);
    }
  }

  this.context.restore();
};

Heatmap.prototype.drawTrace = function(trace, opacity) {
  this.drawTraceLine(trace, 4, calculateColor(255, 255, 255, opacity, 1));
  var color = calculateColor(nthColor(trace)[0], nthColor(trace)[1], nthColor(trace)[2], opacity, 1);
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
  for (var time = 0; time < this.revisions.length; ++time) {
    var values = this.traces[this.revisions[time]][trace];
    var sum = 0;
    for (var value of values)
      sum += value;
    var value = sum / values.length;

    var bucket = mapRange(value, this.min, this.max, 0, this.h);
    if (started) {
      this.context.lineTo(revisionWidth * time, bucket);
    } else {
      this.context.moveTo(revisionWidth * time, bucket);
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
