Heatmap = function(canvas, resolutionSlider, data) {
  this.canvas = canvas.get(0);
  this.context = this.canvas.getContext('2d');
  this.scaleCanvas();

  this.calculate(data);
  this.installInputHandlers(canvas, resolutionSlider);

  this.draw();
};
