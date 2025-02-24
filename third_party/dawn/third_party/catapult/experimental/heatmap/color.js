// Rainbows are hard.
// https://mycarta.wordpress.com/2012/10/06/the-rainbow-is-deadlong-live-the-rainbow-part-3/

var PHI = (1 + Math.sqrt(5)) / 2;

// http://basecase.org/env/on-rainbows
function sinebow(h) {
  h += 0.5;
  h = -h;
  var r = Math.sin(Math.PI * h);
  var g = Math.sin(Math.PI * (h + 1/3));
  var b = Math.sin(Math.PI * (h + 2/3));
  r *= r; g *= g; b *= b;

  // Roughly correct for human perception.
  // https://en.wikipedia.org/wiki/Luma_%28video%29
  // Multiply by 2 to normalize all values to 0.5.
  // (Halfway between black and white.)
  var y = 2 * (0.2989 * r + 0.5870 * g + 0.1140 * b);
  r /= y; g /= y; b /= y;

  return [256 * r, 256 * g, 256 * b];
}

function nthColor(n) {
  return sinebow(n * PHI);
}

function calculateColor(r, g, b, a, brightness) {
  if (brightness <= 1) {
    r *= brightness;
    g *= brightness;
    b *= brightness;
  } else {
    r = mapRange(brightness, 1, 2, r, 255);
    g = mapRange(brightness, 1, 2, g, 255);
    b = mapRange(brightness, 1, 2, b, 255);
  }
  r = Math.round(r);
  g = Math.round(g);
  b = Math.round(b);
  return 'rgba(' + r + ',' + g + ',' + b + ', ' + a + ')';
}
