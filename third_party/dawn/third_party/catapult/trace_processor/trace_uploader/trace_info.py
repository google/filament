# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from google.appengine.ext import ndb


class TraceInfo(ndb.Model):
  config = ndb.StringProperty(indexed=True)
  date = ndb.DateTimeProperty(auto_now_add=True, indexed=True)
  network_type = ndb.StringProperty(indexed=True, default=None)
  prod = ndb.StringProperty(indexed=True)
  remote_addr = ndb.StringProperty(indexed=True)
  tags = ndb.StringProperty(indexed=True, repeated=True)
  user_agent = ndb.StringProperty(indexed=True, default=None)
  ver = ndb.StringProperty(indexed=True)
  scenario_name = ndb.StringProperty(indexed=True, default=None)
  os_name = ndb.StringProperty(indexed=True, default=None)
  os_arch = ndb.StringProperty(indexed=True, default=None)
  os_version = ndb.StringProperty(indexed=True, default=None)
  field_trials = ndb.StringProperty(indexed=True, default=None)
  product_version = ndb.StringProperty(indexed=True, default=None)
  physical_memory = ndb.StringProperty(indexed=True, default=None)
  cpu_brand = ndb.StringProperty(indexed=True, default=None)
  cpu_family = ndb.StringProperty(indexed=True, default=None)
  cpu_stepping = ndb.StringProperty(indexed=True, default=None)
  cpu_model = ndb.StringProperty(indexed=True, default=None)
  num_cpus = ndb.StringProperty(indexed=True, default=None)
  gpu_devid = ndb.StringProperty(indexed=True, default=None)
  gpu_driver = ndb.StringProperty(indexed=True, default=None)
  gpu_psver = ndb.StringProperty(indexed=True, default=None)
  gpu_vsver = ndb.StringProperty(indexed=True, default=None)
  gpu_venid = ndb.StringProperty(indexed=True, default=None)
  highres_ticks = ndb.BooleanProperty(indexed=True, default=True)
