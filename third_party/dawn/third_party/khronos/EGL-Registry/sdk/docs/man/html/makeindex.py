#!/usr/bin/python3
#
# Copyright (c) 2013-2014 The Khronos Group Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Materials.
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

import io, os, re, string, sys;

if __name__ == '__main__':
    if (len(sys.argv) != 5):
        print('Usage:', sys.argv[0], ' gendir srcdir accordfilename flatfilename', file=sys.stderr)
        exit(1)
    else:
        gendir = sys.argv[1]
        srcdir = sys.argv[2]
        accordfilename = sys.argv[3]
        flatfilename = sys.argv[4]
        # print(' gendir = ', gendir, ' srcdir = ', srcdir, 'accordfilename = ', accordfilename, 'flatfilename = ', flatfilename)
else:
    print('Unknown invocation mode', file=sys.stderr)
    exit(1)

# Various levels of indentation in generated HTML
ind1 = '    '
ind2 = ind1 + ind1
ind3 = ind2 + ind1
ind4 = ind2 + ind2

# Symbolic names
notAlias = False
isAlias = True

# Page title
pageTitle = 'EGL Reference Pages'

# Docbook source and generated HTML 5 file extensions
srcext = '.xml'
genext = '.xhtml'

# List of generated files
files = os.listdir(gendir)

# Feature - class representing a command or function to be indexed, used
# as dictionary values keyed by the feature name to be indexed.
#
# Members
#   file - name of file containing the feature
#   feature - feature name for the index (basis for the dictionary key).
#   alias - True if this is an alias of another feature in the file.
#       Usually if alias is False, feature is the basename of file.
#   apiCommand - True if this is an API command, or should be grouped
#       like one
class Feature:
    def __init__(self,
                 file = None,
                 feature = None,
                 alias = False,
                 apiCommand = None):
        self.file = file
        self.feature = feature
        self.alias = alias
        self.apiCommand = apiCommand
        # This is the API-dependent command prefix
        self.prefix = 'egl'
        self.prefixLen = len(self.prefix)
    def makeKey(self):
        # Return dictionary / sort key based on the feature name
        if (self.apiCommand and self.feature[0:self.prefixLen]
              == self.prefix):
            return self.feature[self.prefixLen:]
        else:
            return self.feature

# Add dictionary entry for specified Feature.
# The key used is the feature name, with the leading 'gl' stripped
#  off if this is an API command
def addkey(dict, feature):
    key = feature.makeKey()
    if (key in dict.keys()):
        print('Key', key, ' already exists in dictionary!')
    else:
        dict[key] = feature

# Create list of entry point names to be indexed.
# Unlike the old Perl script, this proceeds as follows:
# - Each .xhtml page with a parent .xml page gets an
#   index entry for its base name.
# - Additionally, each <function> tag inside a <funcdef>
#   in the parent page gets an aliased index entry.
# - Each .xhtml page *without* a parent is reported but
#   not indexed.
# - Each collision in index terms is reported.
# - Index terms are keys in a dictionary whose entries
#   are [ pagename, alias, glPrefix ] where pagename is
#   the base name of the indexed page and alias is True
#   if this index isn't the same as pagename.
# - API keys have their glPrefix value set to True,
#   GLSL keys to False. There is a simplistic way of
#   telling the files apart based on the file name:
#
#   * Everything starting with 'egl[A-Z]' is API
#   * 'removedTypes.*' is API (more may be added)
#   * Everything else is GLSL

def isAPIfile(entrypoint):
    if (re.match('^egl[A-Z]', entrypoint) or entrypoint == 'removedTypes'):
        return True
    else:
        return False

# Dictionary of all keys mapped to Feature values
refIndex = {}

for file in files:
    # print('Processing file', file)
    (entrypoint,ext) = os.path.splitext(file)
    if (ext == genext):
        parent = srcdir + '/' + entrypoint + srcext
        # Determine if this is an API or GLSL page
        apiCommand = isAPIfile(entrypoint)
        if (os.path.exists(parent)):
            addkey(refIndex, Feature(file, entrypoint, False, apiCommand))
            # Search parent file for <function> tags inside <funcdef> tags
            # This doesn't search for <varname> inside <fieldsynopsis>, because
            #   those aren't on the same line and it's hard.
            fp = open(parent)
            for line in fp.readlines():
                # Look for <function> tag contents and add as aliases
                # Don't add the same key twice
                for m in re.finditer(r"<funcdef>.*<function>(.*)</function>.*</funcdef>", line):
                    funcname = m.group(1)
                    if (funcname != entrypoint):
                        addkey(refIndex, Feature(file, funcname, True, apiCommand))
            fp.close()
        else:
            print('No parent page for', file, ', will not be indexed')

# Some utility functions for generating the navigation table
# Opencl_tofc.html uses style.css instead of style-index.css
# flatMenu - if True, don't include accordion JavaScript,
#   generating a flat (expanded) menu.
# letters - if not None, include per-letter links to within
#   the indices for each letter in the list.
# altMenu - if not None, the name of the alternate index to
#   link to.
def printHeader(fp, flatMenu = False, letters = None, altMenu = None):
    if (flatMenu):
        scriptInclude = '    <!-- Don\'t include accord.js -->'
    else:
        scriptInclude = '    <?php include \'accord.js\'; ?>'

    print('<html>',
          '<head>',
          '    <link rel="stylesheet" type="text/css" href="style-index.css" />',
          '    <title>' + pageTitle + '</title>',
               scriptInclude,
          '</head>',
          '<body>',
          sep='\n', file=fp)

    if (altMenu):
        if (flatMenu):
            altLabel = '(accordion-style)'
        else:
            altLabel = '(flat)'
        print('    <a href="' + altMenu + '">' +
              'Use alternate ' + altLabel + ' index' +
              '</a>', file=fp)

    if (letters):
        print('    <center>\n<div id="container">', file=fp)
        for letter in letters:
            print('        <b><a href="#' +
                  letter +
                  '" style="text-decoration:none">' +
                  letter +
                  '</a></b> &nbsp;', file=fp)
        print('    </div>\n</center>', file=fp)

    print('    <div id="navwrap">',
          '    <ul id="containerul"> <!-- Must wrap entire list for expand/contract -->',
          '    <li class="Level1">',
          '        <a href="start.html" target="pagedisplay">Introduction</a>',
          '    </li>',
          sep='\n', file=fp)

def printFooter(fp, flatMenu = False):
    print('    </div> <!-- End containerurl -->', file=fp)
    if (not flatMenu):
        print('    <script type="text/javascript">initiate();</script>', file=fp)
    print('</body>',
          '</html>',
          sep='\n', file=fp)

# Add a nav table entry. key = link name, feature = Feature info for key
def addMenuLink(key, feature, fp):
    file = feature.file
    linkname = feature.feature

    print(ind4 + '<li><a href="' + file + '" target="pagedisplay">'
               + linkname + '</a></li>',
          sep='\n', file=fp)

# Begin index section for a letter, include an anchor to link to
def beginLetterSection(letter, fp):
    print(ind2 + '<a name="' + letter + '"></a>',
          ind2 + '<li>' + letter,
          ind3 + '<ul class="Level3">',
          sep='\n', file=fp)

# End index section for a letter
def endLetterSection(opentable, fp):
    if (opentable == 0):
        return
    print(ind3 + '</ul> <!-- End Level3 -->',
          ind2 + '</li>',
          sep='\n', file=fp)

# Return the keys in a dictionary sorted by name.
# Select only keys matching whichKeys (see genDict below)
def sortedKeys(dict, whichKeys):
    list = []
    for key in dict.keys():
        if (whichKeys == 'all' or
            (whichKeys == 'api' and dict[key].apiCommand) or
            (whichKeys == 'glsl' and not dict[key].apiCommand)):
            list.append(key)
    list.sort(key=str.lower)
    return list

# Generate accordion menu for this dictionary, titled as specified.
#
# If whichKeys is 'all', generate index for all features
# If whichKeys is 'api', generate index only for API features
# If whichKeys is 'glsl', generate index only for GLSL features
#
# fp is the file to write to
def genDict(dict, title, whichKeys, fp):
    print(ind1 + '<li class="Level1">' + title,
          ind2 + '<ul class="Level2">',
          sep='\n', file=fp)

    # Print links for sorted keys in each letter section
    curletter = ''
    opentable = 0

    # Determine which letters are in the table of contents for this
    # dictionary. If apiPrefix is set, strip the API prefix from each
    # key containing it first.

    # Generatesorted list of page indexes. Select keys matching whichKeys.
    keys = sortedKeys(dict, whichKeys)

    # print('@ Sorted list of page indexes:\n', keys)

    for key in keys:
        # Character starting this key
        c = str.lower(key[0])

        if (c != curletter):
            endLetterSection(opentable, fp)
            # Start a new subtable for this letter
            beginLetterSection(c, fp)
            opentable = 1
            curletter = c
        addMenuLink(key, dict[key], fp)
    endLetterSection(opentable, fp)

    print(ind2 + '</ul> <!-- End Level2 -->',
          ind1 + '</li> <!-- End Level1 -->',
          sep='\n', file=fp)

######################################################################

# Generate the accordion menu
fp = open(accordfilename, 'w')
printHeader(fp, flatMenu = False, altMenu = flatfilename)

genDict(refIndex, 'EGL Entry Points', 'all', fp)

printFooter(fp, flatMenu = False)
fp.close()

######################################################################

# Generate the non-accordion menu, with combined API and GLSL sections
fp = open(flatfilename, 'w')

# Set containing all index letters
indices = { key[0].lower() for key in refIndex.keys() }
letters = [c for c in indices]
letters.sort()

printHeader(fp, flatMenu = True, letters = letters, altMenu = accordfilename)

genDict(refIndex, 'EGL Entry Points', 'all', fp)

printFooter(fp, flatMenu = True)
fp.close()
