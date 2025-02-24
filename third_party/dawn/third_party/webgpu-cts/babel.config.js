module.exports = function (api) {
  api.cache(true);
  return {
    presets: ['@babel/preset-typescript'],
    plugins: [
      'const-enum',
      [
        'add-header-comment',
        {
          header: ['AUTO-GENERATED - DO NOT EDIT. Source: https://github.com/gpuweb/cts'],
        },
      ],
    ],
    compact: false,
    // Keeps comments from getting hoisted to the end of the previous line of code.
    // (Also keeps lines close to their original line numbers - but for WPT we
    // reformat with prettier anyway.)
    retainLines: true,
    shouldPrintComment: val => !/eslint|prettier-ignore/.test(val),
  };
};
