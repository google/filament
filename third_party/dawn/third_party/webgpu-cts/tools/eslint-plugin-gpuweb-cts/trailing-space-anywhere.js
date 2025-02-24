module.exports = {
  meta: {
    type: 'suggestion',
    docs: {
      description:
        'Trailing spaces are not allowed, even in multiline strings, due to WPT lint rules.',
    },
    schema: [],
  },
  create: context => {
    const sourceCode = context.getSourceCode();

    return {
      Program: node => {
        for (let lineIdx = 0; lineIdx < sourceCode.lines.length; ++lineIdx) {
          const line = sourceCode.lines[lineIdx];
          const match = /\s+$/.exec(line);
          if (match) {
            context.report({
              node,
              loc: { line: lineIdx + 1, column: match.index },
              message: 'Trailing spaces not allowed.',
              // fixer is hard to implement, so not implemented.
            });
          }
        }
      },
    };
  },
};
