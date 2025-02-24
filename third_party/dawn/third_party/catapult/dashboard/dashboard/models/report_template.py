# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import functools

from google.appengine.ext import ndb

from dashboard.common import report_query
from dashboard.common import timing
from dashboard.common import utils
from dashboard.models import internal_only_model
import six


class ReportTemplate(internal_only_model.InternalOnlyModel):
  internal_only = ndb.BooleanProperty(indexed=True, default=False)
  name = ndb.StringProperty()
  modified = ndb.DateTimeProperty(indexed=False, auto_now=True)
  owners = ndb.StringProperty(repeated=True)
  template = ndb.JsonProperty()


STATIC_TEMPLATES = []


def ListStaticTemplates():
  return [
      handler for handler in STATIC_TEMPLATES
      if (not handler.template.internal_only) or utils.IsInternalUser()
  ]


def Static(internal_only, template_id, name, modified):
  """Register a static report template handler function.

  This decorator is a bit fiddly! You may prefer creating ReportTemplate
  entities if possible using [Create Report Template] in the UI or PutTemplate()
  either in dev console. They automatically handle id, modified, and
  internal_only, and they support /api/alerts?report whereas static templates do
  not. However, static template handler functions are version-controlled and
  fully programmable.

  Tips:
  - Use this code to generate `template_id` and `modified` and paste them into
    your code:
      import random, datetime
      print 'template_id=%d' % int(random.random() * (2 ** 31))
      print 'modified=%r' % datetime.datetime.now()
  - Put your static templates in common/ and make api/report_names.py and
    api/report_generate.py import it so that it is run.
  - The generated report must specify a documentation url at template['url'].
  - RegisterStaticTemplate() may provide further syntactic sugar.
  - If you want to dynamically fetch available bots or test cases, see
    update_test_suite_descriptors.FetchCachedTestSuiteDescriptor().
    The cached test suite descriptors are regenerated once per day, but you
    control when you fetch the descriptor: you can either call it when your
    module is imported, or when your static template handler function is called.
    Your module is imported whenever app engine spins up a new instance, which
    happens when we deploy a new version or when a burst of traffic requires a
    new instance. Your static template handler function is called when a client
    hits /api/report/generate. Fetching the descriptor is a single database
    get() which is usually <100ms, so if you're at all concerned about using
    stale data, then I'd recommend fetching the descriptor and building the
    template in your handler function when the client hits /api/report/generate.

  Usage:
  @report_template.Static(
      internal_only=False,
      template_id=1797699531,
      name='Paryl:Awesome:Report',
      modified=datetime.datetime(2018, 8, 2))
  def ParylAwesomeReport(revisions):
    desc = update_test_suite_descriptors.FetchCachedTestSuiteDescriptor('paryl')
    template = MakeAwesomeTemplate(desc)
    template['url'] = 'https://example.com/your/documentation.html'
    return report_query.ReportQuery(template, revisions)

  Names need to be mutable and are for display purposes only. Identifiers need
  to be immutable and are used for caching. Ids are required to be ints so the
  API can validate them and so they convey a sense of "danger, don't change
  this".
  """
  assert isinstance(template_id, int)  # JS can't handle python floats or longs!

  def Decorator(decorated):

    @functools.wraps(decorated)
    def Replacement(revisions):
      report = decorated(revisions)
      if isinstance(report, report_query.ReportQuery):
        report = report.FetchSync()
      assert isinstance(
          report.get('url'),
          six.string_types), ('Reports are required to link to documentation')
      return report

    Replacement.template = ReportTemplate(
        internal_only=internal_only,
        id=template_id,
        name=name,
        modified=modified)
    STATIC_TEMPLATES.append(Replacement)
    return Replacement

  return Decorator


def List():
  with timing.WallTimeLogger('List'), timing.CpuTimeLogger('List'):
    templates = ReportTemplate.query().fetch()
    templates += [handler.template for handler in ListStaticTemplates()]
    templates = [{
        'id': template.key.id(),
        'name': template.name,
        'modified': template.modified.isoformat(),
    } for template in templates]
    return sorted(templates, key=lambda d: d['name'])


def PutTemplate(template_id, name, owners, template):
  email = utils.GetEmail()
  if email is None:
    raise ValueError
  if template_id is None:
    if any(name == existing['name'] for existing in List()):
      raise ValueError
    entity = ReportTemplate()
  else:
    for handler in STATIC_TEMPLATES:
      if handler.template.key.id() == template_id:
        raise ValueError
    try:
      entity = ndb.Key('ReportTemplate', template_id).get()
    except AssertionError as e:
      # InternalOnlyModel._post_get_hook asserts that the user can access the
      # entity.
      six.raise_from(ValueError, e)
    if not entity or email not in entity.owners:
      raise ValueError
    if any(name == existing['name']
           for existing in List()
           if existing['id'] != template_id):
      raise ValueError

  entity.internal_only = _GetInternalOnly(template)
  entity.name = name
  entity.owners = owners
  entity.template = template
  entity.put()


def _GetInternalOnly(template):
  futures = []
  for table_row in template['rows']:
    for desc in report_query.TableRowDescriptors(table_row):
      for test_path in desc.ToTestPathsSync():
        futures.append(utils.TestMetadataKey(test_path).get_async())
      desc.statistic = 'avg'
      for test_path in desc.ToTestPathsSync():
        futures.append(utils.TestMetadataKey(test_path).get_async())
  ndb.Future.wait_all(futures)
  tests = [future.get_result() for future in futures]
  return any(test.internal_only for test in tests if test)


def GetReport(template_id, revisions):
  with timing.WallTimeLogger('GetReport'), timing.CpuTimeLogger('GetReport'):
    try:
      template = ndb.Key('ReportTemplate', template_id).get()
    except AssertionError:
      # InternalOnlyModel._post_get_hook asserts that the user can access the
      # entity.
      return None

    result = {'editable': False}
    if template:
      result['owners'] = template.owners
      result['editable'] = utils.GetEmail() in template.owners
      result['report'] = report_query.ReportQuery(template.template,
                                                  revisions).FetchSync()
    else:
      for handler in ListStaticTemplates():
        if handler.template.key.id() != template_id:
          continue
        template = handler.template
        report = handler(revisions)
        if isinstance(report, report_query.ReportQuery):
          report = report.FetchSync()
        result['report'] = report
        break
      if template is None:
        return None

    result['id'] = template.key.id()
    result['name'] = template.name
    result['internal'] = template.internal_only
    return result


def TestKeysForReportTemplate(template_id):
  template = ndb.Key('ReportTemplate', int(template_id)).get()
  if not template:
    return

  for table_row in template.template['rows']:
    for desc in report_query.TableRowDescriptors(table_row):
      for test_path in desc.ToTestPathsSync():
        yield utils.TestMetadataKey(test_path)
        yield utils.OldStyleTestKey(test_path)
