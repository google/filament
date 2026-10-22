# Copyright 2017-2022 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

# Generate the PHP index file includes for OpenGL ARB, OpenGL ES, and
# OpenGL EXT/Vendor extensions.

# python3 is required.

PYTHON = python3
SCRIPTS = registry.py makeindex.py
TARGETS = arbext.php esext.php glext.php scext.php

all: $(TARGETS)

arbext.php: $(SCRIPTS)
	$(PYTHON) makeindex.py arbnumber >$@

esext.php: $(SCRIPTS)
	$(PYTHON) makeindex.py esnumber >$@

glext.php: $(SCRIPTS)
	$(PYTHON) makeindex.py number >$@

scext.php: $(SCRIPTS)
	$(PYTHON) makeindex.py scnumber >$@

clean:
	-rm -f $(TARGETS)
