# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Setups the Django test environment and provides helper classes."""

import django
from django import test
from django.contrib.sessions.backends.file import SessionStore
from django.test.runner import DiscoverRunner

django.setup()
default_app_config = 'tests.contrib.django_util.apps.AppConfig'


class TestWithDjangoEnvironment(test.TestCase):
    @classmethod
    def setUpClass(cls):
        django.setup()
        cls.runner = DiscoverRunner()
        cls.runner.setup_test_environment()
        cls.old_config = cls.runner.setup_databases()

    @classmethod
    def tearDownClass(cls):
        cls.runner.teardown_databases(cls.old_config)
        cls.runner.teardown_test_environment()

    def setUp(self):
        self.factory = test.RequestFactory()

        store = SessionStore()
        store.save()
        self.session = store
