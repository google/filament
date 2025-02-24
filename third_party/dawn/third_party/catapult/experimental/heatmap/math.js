function constrain(value, low, high) {
  if (value < low)
    return low;
  if (value > high)
    return high;
  return value;
}

function mapRange(value, start1, stop1, start2, stop2) {
  return (value - start1) / (stop1 - start1) * (stop2 - start2) + start2;
}

function percentile(values, percentile) {
  var cutoff = values.length * percentile;
  return values.slice(cutoff, cutoff + 1)[0];
}
