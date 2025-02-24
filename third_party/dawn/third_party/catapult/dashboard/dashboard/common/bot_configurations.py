# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from google.appengine.ext import ndb

from dashboard.common import namespaced_stored_object

BOT_CONFIGURATIONS_KEY = 'bot_configurations'


def Get(name):
  configurations = namespaced_stored_object.Get(BOT_CONFIGURATIONS_KEY) or {}
  configuration = configurations.get(name)
  if configuration is None:
    raise ValueError('Bot configuration not found: "%s"' % (name,))
  if 'alias' in configuration:
    return configurations[configuration['alias']]
  return configuration


@ndb.tasklet
def GetAliasesAsync(bot):
  aliases = {bot}
  configurations = yield namespaced_stored_object.GetAsync(
      BOT_CONFIGURATIONS_KEY)
  if not configurations or bot not in configurations:
    raise ndb.Return(aliases)
  if 'alias' in configurations[bot]:
    bot = configurations[bot]['alias']
    aliases.add(bot)
  for name, configuration in configurations.items():
    if configuration.get('alias') == bot:
      aliases.add(name)
  raise ndb.Return(aliases)


def List():
  bot_configurations = namespaced_stored_object.Get(BOT_CONFIGURATIONS_KEY)
  if not bot_configurations:
    return []
  canonical_names = [
      name for name, value in bot_configurations.items() if 'alias' not in value
  ]
  return sorted(canonical_names, key=str.lower)
