# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import nox

BLACK_VERSION = "black==19.10b0"
BLACK_PATHS = ["google_auth_httplib2.py", "tests"]

TEST_DEPENDENCIES = [
    "flask",
    "pytest",
    "pytest-cov",
    "pytest-localserver",
    "httplib2",
]

DEFAULT_PYTHON_VERSION = "3.8"
UNIT_TEST_PYTHON_VERSIONS = ["3.6", "3.7", "3.8", "3.9", "3.10", "3.11", "3.12"]


@nox.session(python=DEFAULT_PYTHON_VERSION)
def lint(session):
    session.install("flake8", "flake8-import-order", "docutils", BLACK_VERSION, "click<8.1.0")
    session.install(".")
    session.run("black", "--check", *BLACK_PATHS)
    session.run(
        "flake8",
        "--import-order-style=google",
        "--application-import-names=google_auth_httplib2,tests",
        *BLACK_PATHS,
    )
    session.run(
        "python", "setup.py", "check", "--metadata", "--restructuredtext", "--strict"
    )


@nox.session(python="3.6")
def blacken(session):
    """Run black.

    Format code to uniform standard.

    This currently uses Python 3.6 due to the automated Kokoro run of synthtool.
    That run uses an image that doesn't have 3.6 installed. Before updating this
    check the state of the `gcp_ubuntu_config` we use for that Kokoro run.
    """
    session.install(BLACK_VERSION, "click<8.1.0")
    session.run("black", *BLACK_PATHS)


@nox.session(python=UNIT_TEST_PYTHON_VERSIONS)
def unit(session):
    session.install(*TEST_DEPENDENCIES)
    session.install(".")
    session.run("pytest", "--cov=google_auth_httplib2", "--cov=tests", "tests")


@nox.session(python=DEFAULT_PYTHON_VERSION)
def cover(session):
    session.install(*TEST_DEPENDENCIES)
    session.install(".")
    session.run(
        "pytest", "--cov=google_auth_httplib2", "--cov=tests", "--cov-report=", "tests"
    )
    session.run("coverage", "report", "--show-missing", "--fail-under=100")

