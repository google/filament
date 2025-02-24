"""Demonstrate how different parsers parse the same markup.

Beautiful Soup can use any of a number of different parsers. Every
parser should behave more or less the same on valid markup, and
Beautiful Soup's unit tests make sure this is the case. But every
parser handles invalid markup differently. Even different versions of
the same parser handle invalid markup differently. So instead of unit
tests I've created this educational demonstration script.

The file demonstration_markup.txt contains many lines of HTML. This
script tests each line of markup against every parser you have
installed, and prints out how each parser sees that markup. This may
help you choose a parser, or understand why Beautiful Soup presents
your document the way it does.
"""

import os
import sys
from bs4 import BeautifulSoup
parsers = ['html.parser']

try:
    from bs4.builder import _lxml
    parsers.append('lxml')
except ImportError, e:
    pass

try:
    from bs4.builder import _html5lib
    parsers.append('html5lib')
except ImportError, e:
    pass

class Demonstration(object):
    def __init__(self, markup):
        self.results = {}
        self.markup = markup

    def run_against(self, *parser_names):
        uniform_results = True
        previous_output = None
        for parser in parser_names:
            try:
                soup = BeautifulSoup(self.markup, parser)
                if markup.startswith("<div>"):
                    # Extract the interesting part
                    output = soup.div
                else:
                    output = soup
            except Exception, e:
                output = "[EXCEPTION] %s" % str(e)
            self.results[parser] = output
            if previous_output is None:
                previous_output = output
            elif previous_output != output:
                uniform_results = False
        return uniform_results

    def dump(self):
        print "%s: %s" % ("Markup".rjust(13), self.markup.encode("utf8"))
        for parser, output in self.results.items():
            print "%s: %s" % (parser.rjust(13), output.encode("utf8"))

different_results = []
uniform_results = []

print "= Testing the following parsers: %s =" % ", ".join(parsers)
print

input_file = sys.stdin
if sys.stdin.isatty():
    for filename in [
        "demonstration_markup.txt",
        os.path.join("scripts", "demonstration_markup.txt")]:
        if os.path.exists(filename):
            input_file = open(filename)

for markup in input_file:
    demo = Demonstration(markup.decode("utf8").strip().replace("\\n", "\n"))
    is_uniform = demo.run_against(*parsers)
    if is_uniform:
        uniform_results.append(demo)
    else:
        different_results.append(demo)

print "== Markup that's handled the same in every parser =="
print
for demo in uniform_results:
    demo.dump()
    print
print "== Markup that's not handled the same in every parser =="
print
for demo in different_results:
    demo.dump()
    print
