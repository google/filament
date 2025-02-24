/**
 * The interface used for formatting strings to contain color metadata.
 *
 * Use the interface properties to construct a style, then use the
 * `(s: string): string` function to format the provided string with the given
 * style.
 */
export interface Colors {
  // Are colors enabled?
  enabled: boolean;

  // Returns the string formatted to contain the specified color or style.
  (s: string): string;

  // modifiers
  reset: Colors;
  bold: Colors;
  dim: Colors;
  italic: Colors;
  underline: Colors;
  inverse: Colors;
  hidden: Colors;
  strikethrough: Colors;

  // colors
  black: Colors;
  red: Colors;
  green: Colors;
  yellow: Colors;
  blue: Colors;
  magenta: Colors;
  cyan: Colors;
  white: Colors;
  gray: Colors;
  grey: Colors;

  // bright colors
  blackBright: Colors;
  redBright: Colors;
  greenBright: Colors;
  yellowBright: Colors;
  blueBright: Colors;
  magentaBright: Colors;
  cyanBright: Colors;
  whiteBright: Colors;

  // background colors
  bgBlack: Colors;
  bgRed: Colors;
  bgGreen: Colors;
  bgYellow: Colors;
  bgBlue: Colors;
  bgMagenta: Colors;
  bgCyan: Colors;
  bgWhite: Colors;

  // bright background colors
  bgBlackBright: Colors;
  bgRedBright: Colors;
  bgGreenBright: Colors;
  bgYellowBright: Colors;
  bgBlueBright: Colors;
  bgMagentaBright: Colors;
  bgCyanBright: Colors;
  bgWhiteBright: Colors;
}

/**
 * The interface used for formatting strings with color metadata.
 *
 * Currently Colors will use the 'ansi-colors' module if it can be loaded.
 * If it cannot be loaded, then the Colors implementation is a straight pass-through.
 *
 * Colors may also be a no-op if the current environment does not support colors.
 */
export let Colors: Colors;

try {
  /* eslint-disable-next-line node/no-unpublished-require, n/no-restricted-require */
  Colors = require('ansi-colors') as Colors;
} catch {
  const passthrough = ((s: string) => s) as Colors;
  passthrough.enabled = false;
  passthrough.reset = passthrough;
  passthrough.bold = passthrough;
  passthrough.dim = passthrough;
  passthrough.italic = passthrough;
  passthrough.underline = passthrough;
  passthrough.inverse = passthrough;
  passthrough.hidden = passthrough;
  passthrough.strikethrough = passthrough;
  passthrough.black = passthrough;
  passthrough.red = passthrough;
  passthrough.green = passthrough;
  passthrough.yellow = passthrough;
  passthrough.blue = passthrough;
  passthrough.magenta = passthrough;
  passthrough.cyan = passthrough;
  passthrough.white = passthrough;
  passthrough.gray = passthrough;
  passthrough.grey = passthrough;
  passthrough.blackBright = passthrough;
  passthrough.redBright = passthrough;
  passthrough.greenBright = passthrough;
  passthrough.yellowBright = passthrough;
  passthrough.blueBright = passthrough;
  passthrough.magentaBright = passthrough;
  passthrough.cyanBright = passthrough;
  passthrough.whiteBright = passthrough;
  passthrough.bgBlack = passthrough;
  passthrough.bgRed = passthrough;
  passthrough.bgGreen = passthrough;
  passthrough.bgYellow = passthrough;
  passthrough.bgBlue = passthrough;
  passthrough.bgMagenta = passthrough;
  passthrough.bgCyan = passthrough;
  passthrough.bgWhite = passthrough;
  passthrough.bgBlackBright = passthrough;
  passthrough.bgRedBright = passthrough;
  passthrough.bgGreenBright = passthrough;
  passthrough.bgYellowBright = passthrough;
  passthrough.bgBlueBright = passthrough;
  passthrough.bgMagentaBright = passthrough;
  passthrough.bgCyanBright = passthrough;
  passthrough.bgWhiteBright = passthrough;
  Colors = passthrough;
}
