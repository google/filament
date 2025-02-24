module.exports = {
  meta: {
    type: 'suggestion',
    docs: {
      description:
        'Indentation tabs are not allowed, even in multiline strings, due to WPT lint rules. This rule simply disallows tabs anywhere.',
    },
    schema: [],
  },
  create: context => {
    const sourceCode = context.getSourceCode();

    return {
      Program: node => {
        for (let lineIdx = 0; lineIdx < sourceCode.lines.length; ++lineIdx) {
          const line = sourceCode.lines[lineIdx];
          const matches = line.matchAll(/\t/g);
          for (const match of matches) {
            context.report({
              node,
              loc: { line: lineIdx + 1, column: match.index },
              message: 'Tabs not allowed.',
              // fixer is hard to implement, so not implemented.
            });
          }
        }
      },
    };
  },
};
