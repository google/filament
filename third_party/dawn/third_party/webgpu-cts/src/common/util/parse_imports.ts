/**
 * Parses all the paths of the typescript `import` statements from content
 * @param path the current path of the file
 * @param content the file content
 * @returns the list of import paths
 */
export function parseImports(path: string, content: string): string[] {
  const out: string[] = [];
  const importRE = /^import\s[^'"]*(['"])([./\w]*)(\1);/gm;
  let importMatch: RegExpMatchArray | null;
  while ((importMatch = importRE.exec(content))) {
    const importPath = importMatch[2].replace(`'`, '').replace(`"`, '');
    out.push(joinPath(path, importPath));
  }
  return out;
}

function joinPath(a: string, b: string): string {
  const aParts = a.split('/');
  const bParts = b.split('/');
  aParts.pop(); // remove file
  let bStart = 0;
  while (aParts.length > 0) {
    switch (bParts[bStart]) {
      case '.':
        bStart++;
        continue;
      case '..':
        aParts.pop();
        bStart++;
        continue;
    }
    break;
  }
  return [...aParts, ...bParts.slice(bStart)].join('/');
}
