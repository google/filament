# 1. convert tables to use 'tabu'
# 2. always add hypertargets, before headings, to workaround issue #2719
# Based on Wagner Macedo's filter.py posted at
# https://groups.google.com/forum/#!msg/pandoc-discuss/RUC-tuu_qf0/h-H3RRVt1coJ
import pandocfilters as pf

def latex(s):
    return pf.RawBlock('latex', s)

def inlatex(s):
    return pf.RawInline('latex', s)

def tbl_caption(s):
    return pf.Para([inlatex(r'\caption{')] + s + [inlatex(r'}')])

def tbl_alignment(a, w):
    aligns = {
        "AlignDefault": 'l',
        "AlignLeft": 'l',
        "AlignCenter": 'c',
        "AlignRight": 'r',
    }
    s = '';
    for i in range(len(a)):
        s += 'X[%.3f,' % -w[i] + aligns[a[i]['t']] + ']'
    return s;

def tbl_headers(s):
    result = s[0][0]['c'][:]
    for i in range(1, len(s)):
        result.append(inlatex(' & '))
        result.extend(s[i][0]['c'])
    result.append(inlatex(r'\\' '\n'))
    return pf.Para(result)

def tbl_contents(s):
    result = []
    for row in s:
        para = []
        for col in row:
            if col:
                para.extend(col[0]['c'])
            para.append(inlatex(' & '))
        result.extend(para)
        result[-1] = inlatex(r'\\' '\n')
    return pf.Para(result)

def do_filter(k, v, f, m):
    if k == "Table":
        w = v[2]
        if sum(w) == 0:
            w = [1 for e in w]
            wd = ''
            ha = r'\centering'
        else:
            wd = '*'
            ha = r'\raggedright'
        return [latex(r'\begin{table'+wd+'}[!h]'),
                tbl_caption(v[0]),
                latex(ha),
                latex(r'\begin{tabu} spread 0pt {' + tbl_alignment(v[1], w) + '}'),
                latex(r'\toprule'),
                tbl_headers(v[3]),
                latex(r'\midrule'),
                tbl_contents(v[4]),
                latex(r'\bottomrule' '\n' r'\end{tabu}'),
                latex(r'\end{table'+wd+'}')]
    if k == "Header":
        return [latex(r'\hypertarget{' + v[1][0] + r'}{}'),
                pf.Header(v[0], v[1], v[2])]

if __name__ == "__main__":
    pf.toJSONFilter(do_filter)
