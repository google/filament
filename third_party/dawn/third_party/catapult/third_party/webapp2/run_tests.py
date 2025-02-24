import os
import sys
import unittest

gae_path = '/usr/local/google_appengine'

current_path = os.path.abspath(os.path.dirname(__file__))
tests_path = os.path.join(current_path, 'tests')
sys.path[0:0] = [
    current_path,
    tests_path,
    gae_path,
    # All libs used by webapp2 and extras.
    os.path.join(current_path, 'lib', 'babel'),
    os.path.join(current_path, 'lib', 'Jinja2-2.6'),
    os.path.join(current_path, 'lib', 'Mako-0.4.1'),
    os.path.join(current_path, 'lib', 'gaepytz-2011h'),
    os.path.join(current_path, 'lib', 'WebOb-1.0.8'),
    # SDK libs.
    os.path.join(gae_path, 'lib', 'django_0_96'),
    #os.path.join(gae_path, 'lib', 'webob'),
    os.path.join(gae_path, 'lib', 'yaml', 'lib'),
    os.path.join(gae_path, 'lib', 'protorpc'),
    os.path.join(gae_path, 'lib', 'simplejson'),
]

all_tests = [f[:-8] for f in os.listdir(tests_path) if f.endswith('_test.py')]

def get_suite(tests):
    tests = sorted(tests)
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    for test in tests:
        suite.addTest(loader.loadTestsFromName(test))
    return suite

if __name__ == '__main__':
    """
    To run all tests:
        $ python run_tests.py
    To run a single test:
        $ python run_tests.py app
    To run a couple of tests:
        $ python run_tests.py app config sessions
    To run code coverage:
        $ coverage run run_tests.py
        $ coverage report -m
    """
    tests = sys.argv[1:]
    if not tests:
        tests = all_tests
    tests = ['%s_test' % t for t in tests]
    suite = get_suite(tests)
    unittest.TextTestRunner(verbosity=2).run(suite)
