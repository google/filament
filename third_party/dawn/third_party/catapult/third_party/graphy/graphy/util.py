def _IsColor(color):
  """Try to determine if color is a hex color string.  
  Labels that look like hex colors will match too, unfortunately."""
  if not isinstance(color, basestring):
    return False
  color = color.strip('#')
  if len(color) != 3 and len(color) != 6:
    return False
  hex_letters = '0123456789abcdefABCDEF'
  for letter in color:
    if letter not in hex_letters:
      return False
  return True
