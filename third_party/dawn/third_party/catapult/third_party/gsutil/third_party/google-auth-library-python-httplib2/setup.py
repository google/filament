# Copyright 2014 Google Inc.
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

import io

from setuptools import setup

version = "0.2.0"

DEPENDENCIES = ["google-auth", "httplib2 >= 0.19.0"]


with io.open("README.rst", "r") as fh:
    long_description = fh.read()


setup(
    name="google-auth-httplib2",
    version=version,
    author="Google Cloud Platform",
    author_email="googleapis-packages@google.com",
    description="Google Authentication Library: httplib2 transport",
    long_description=long_description,
    url="https://github.com/GoogleCloudPlatform/google-auth-library-python-httplib2",
    py_modules=["google_auth_httplib2"],
    install_requires=DEPENDENCIES,
    license="Apache 2.0",
    keywords="google auth oauth client",
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: POSIX",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: MacOS :: MacOS X",
        "Operating System :: OS Independent",
        "Topic :: Internet :: WWW/HTTP",
    ],
)
