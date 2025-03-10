package android.dawn.helper

/**
 * Helpers for image dimension management.  Each adds a method to round to the next higher
 * or lower multiple of a value, e.g., "round -1,234 down to the nearest multiple of 1,000"
 * returns -2000.
 */
public fun Long.roundDownToNearestMultipleOf(boundary: Int): Long {
  val padding = if (this < 0 && (this % boundary != 0L)) boundary else 0
  return ((this - padding) / boundary) * boundary
}

public fun Int.roundDownToNearestMultipleOf(boundary: Int): Int {
  val padding = if (this < 0 && (this % boundary != 0)) boundary else 0
  return ((this - padding) / boundary) * boundary
}

public fun Long.roundUpToNearestMultipleOf(boundary: Int): Long {
  val padding = if (this > 0 && (this % boundary != 0L)) boundary else 0
  return ((this + padding) / boundary) * boundary
}

public fun Int.roundUpToNearestMultipleOf(boundary: Int): Int {
  val padding = if (this > 0 && (this % boundary != 0)) boundary else 0
  return ((this + padding) / boundary) * boundary
}
