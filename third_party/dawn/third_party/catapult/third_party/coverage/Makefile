# Licensed under the Apache License: http://www.apache.org/licenses/LICENSE-2.0
# For details: https://bitbucket.org/ned/coveragepy/src/default/NOTICE.txt

# Makefile for utility work on coverage.py.

default:
	@echo "* No default action *"

clean:
	-rm -f *.pyd */*.pyd
	-rm -f *.so */*.so
	-PYTHONPATH=. python tests/test_farm.py clean
	-rm -rf build coverage.egg-info dist htmlcov
	-rm -f *.pyc */*.pyc */*/*.pyc */*/*/*.pyc */*/*/*/*.pyc */*/*/*/*/*.pyc
	-rm -f *.pyo */*.pyo */*/*.pyo */*/*/*.pyo */*/*/*/*.pyo */*/*/*/*/*.pyo
	-rm -f *.bak */*.bak */*/*.bak */*/*/*.bak */*/*/*/*.bak */*/*/*/*/*.bak
	-rm -f *$$py.class */*$$py.class */*/*$$py.class */*/*/*$$py.class */*/*/*/*$$py.class */*/*/*/*/*$$py.class
	-rm -rf __pycache__ */__pycache__ */*/__pycache__ */*/*/__pycache__ */*/*/*/__pycache__ */*/*/*/*/__pycache__
	-rm -f coverage/*,cover
	-rm -f MANIFEST
	-rm -f .coverage .coverage.* coverage.xml .metacov* .noseids
	-rm -f tests/zipmods.zip
	-rm -rf tests/eggsrc/build tests/eggsrc/dist tests/eggsrc/*.egg-info
	-rm -f setuptools-*.egg distribute-*.egg distribute-*.tar.gz
	-rm -rf doc/_build doc/_spell

sterile: clean
	-rm -rf .tox*

LINTABLE = coverage igor.py setup.py tests ci/*.py

lint:
	-pylint $(LINTABLE)
	python -m tabnanny $(LINTABLE)
	python igor.py check_eol

spell:
	-pylint --disable=all --enable=spelling $(LINTABLE)

pep8:
	pep8 --filename=*.py --repeat $(LINTABLE)

test:
	tox -e py27,py34 $(ARGS)

metacov:
	COVERAGE_COVERAGE=yes tox $(ARGS)

metahtml:
	python igor.py combine_html

# Kitting

kit:
	python setup.py sdist --formats=gztar,zip

wheel:
	tox -c tox_wheels.ini $(ARGS)

kit_upload:
	twine upload dist/*

kit_local:
	cp -v dist/* `awk -F "=" '/find-links/ {print $$2}' ~/.pip/pip.conf`
	# pip caches wheels of things it has installed. Clean them out so we
	# don't go crazy trying to figure out why our new code isn't installing.
	find ~/Library/Caches/pip/wheels -name 'coverage-*' -delete

download_appveyor:
	python ci/download_appveyor.py nedbat/coveragepy

pypi:
	python setup.py register

build_ext:
	python setup.py build_ext

install:
	python setup.py install

uninstall:
	-rm -rf $(PYHOME)/lib/site-packages/coverage*
	-rm -rf $(PYHOME)/scripts/coverage*

# Documentation

SPHINXBUILD = sphinx-build
SPHINXOPTS = -a -E doc
WEBHOME = ~/web/stellated/
WEBSAMPLE = $(WEBHOME)/files/sample_coverage_html
WEBSAMPLEBETA = $(WEBHOME)/files/sample_coverage_html_beta

docreqs:
	pip install -r doc/requirements.pip

dochtml:
	$(SPHINXBUILD) -b html $(SPHINXOPTS) doc/_build/html
	@echo
	@echo "Build finished. The HTML pages are in doc/_build/html."

docspell:
	$(SPHINXBUILD) -b spelling $(SPHINXOPTS) doc/_spell

publish:
	rm -f $(WEBSAMPLE)/*.*
	mkdir -p $(WEBSAMPLE)
	cp doc/sample_html/*.* $(WEBSAMPLE)

publishbeta:
	rm -f $(WEBSAMPLEBETA)/*.*
	mkdir -p $(WEBSAMPLEBETA)
	cp doc/sample_html_beta/*.* $(WEBSAMPLEBETA)
