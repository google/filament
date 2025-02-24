# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Helpers for handling Buganizer migration.

Monorail is being deprecated and replaced by Buganizer in 2024. Migration
happens by projects. Chromeperf needs to support both Monorail and Buganizer
during the migration until the last project we supported is fully migrated.

Before the first migration happens, we will keep the consumers untouched
in the Monorail fashion. The requests and responses to and from Buganizer will
be reconciled to the Monorail format on perf_issue_service.
"""

import logging

from application.clients import monorail_client

# ============ mapping helpers start ============
# The mapping here are for ad hoc testing. The real mappings will be different
# from project to project and will be added in future CLs.

# components, labels and hotlists for testing:
# Buganizer component 1325852 is "ChromePerf testing"
# Monorail labels "chromeperf-test" and "chromeperf-test-2"
# Buganizer hotlists 5141966 and 5142065

# Mappings from a monorail component to a buganizer component id.
COMPONENT_MAP_CR2B = {
  # Chromium
  'Blink>Accessibility': 1456587,
  'Blink>Bindings': 1456743,
  'Blink>CSS': 1456329,
  'Blink>DOM': 1456718,
  'Blink>DOM>ShadowDOM': 1456822,
  'Blink>HTML>Parser': 1456175,
  'Blink>JavaScript': 1456824,
  'Blink>JavaScript>GarbageCollection': 1456490,
  'Blink>JavaScript>WebAssembly': 1456332,
  'Blink>Layout': 1456721,
  'Blink>Paint': 1456440,
  'Blink>ServiceWorker': 1456150,
  'Blink>SVG': 1456414,
  'Blink>Storage': 1456519,
  'Blink>Storage>IndexedDB': 1456771,
  'Blink>WebRTC>Perf': 1456207,
  'Fuchsia': 1456675,
  'Internals>GPU>ANGLE': 1456215,
  'Internals>GPU>Metrics': 1456424,
  'Internals>Network>Library': 1456244,
  'Internals>Power': 1456482,
  'Internals>Skia': 1457031,
  'Internals>XR': 1456683,
  'Mobile>Fundamentals>SystemHealth': 1457150,
  'Platform>Apps>ARC': 1457293,
  'Platform>DevTools>WebAssembly': 1456350,
  'Speed>BinarySize>Android': 1457059,
  'Speed>BinarySize>Desktop': 1456597,
  'Speed>Dashboard': 1457390,
  'Speed>Regressions': 1457332,
  'Test>Telemetry': 1456742,
  'Speed>Tracing': 1457213,
  'UI>Browser>AdFilter': 1456114,
  'UI>Browser>NewTabPage': 1457163,
  'UI>Browser>Omnibox': 1457180,
  'UI>Browser>TopChrome': 1457234,
  # A new component is required in Buganizer after migration.
  # This mapping is used for migration time and will be removed.
  'Internals>Media': 1456190,

  # Fuchsia
  # Trackers > Fuchsia > UntriagedPerformanceAlerts
  'UntriagedPerformanceAlerts': 1454999,

  # WebRTC
  'Audio': 1565681,
  'Perf>Arcturus': 1565993,
  'Perf>Canopus': 1565890,
  'Perf>Migration': 1565319,
  'Perf>Video': 1565864,
  'Stats': 1565682,

  # Test only. Remove after migration.
  'ChromePerf testing': 1325852
}

# Mappings from a buganizer component id to a monorail project
COMPONENT_MAP_B2CR = {
  # Chromium
  '1456587': 'chromium',
  '1456743': 'chromium',
  '1456329': 'chromium',
  '1456718': 'chromium',
  '1456822': 'chromium',
  '1456175': 'chromium',
  '1456824': 'chromium',
  '1456490': 'chromium',
  '1456332': 'chromium',
  '1456721': 'chromium',
  '1456440': 'chromium',
  '1456414': 'chromium',
  '1456519': 'chromium',
  '1456771': 'chromium',
  '1456207': 'chromium',
  '1456675': 'chromium',
  '1456215': 'chromium',
  '1456424': 'chromium',
  '1456244': 'chromium',
  '1456482': 'chromium',
  '1457031': 'chromium',
  '1456683': 'chromium',
  '1457150': 'chromium',
  '1457293': 'chromium',
  '1456350': 'chromium',
  '1457059': 'chromium',
  '1456597': 'chromium',
  '1457332': 'chromium',
  '1456742': 'chromium',
  '1457213': 'chromium',
  '1456114': 'chromium',
  '1457163': 'chromium',
  '1457180': 'chromium',
  '1457234': 'chromium',
  '1456150': 'chromium',
  '1456190': 'chromium',
  '1457390': 'chromium',

  # Fuchsia
  '1454999' : 'fuchsia',

  # webrtc
  '1565681': 'webrtc',
  '1565993': 'webrtc',
  '1565890': 'webrtc',
  '1565319': 'webrtc',
  '1565864': 'webrtc',
  '1565682': 'webrtc',

  # Test only. Remove after migration.
  '1325852' : 'MigratedProject'
}

# Mapping from monorail project to all the related components in buganizer
PROJECT_MAP_CR2B = {
  # Chromium
  'chromium': [
    '1456114', '1456150', '1456175', '1456207', '1456215', '1456244', '1456329',
    '1456332', '1456350', '1456414', '1456424', '1456440', '1456482', '1456490',
    '1456519', '1456587', '1456597', '1456675', '1456683', '1456718', '1456721',
    '1456742', '1456743', '1456771', '1456822', '1456824', '1457031', '1457059',
    '1457150', '1457163', '1457180', '1457213', '1457234', '1457293', '1457332',
    '1456190', '1457390'
    ],

  # Fuchsia
  'fuchsia': ['1454999'],

  # webrtc
  'webrtc': ['1565681', '1565993', '1565890', '1565319', '1565864', '1565682'],

  # Test only. Remove after migration.
  'MigratedProject': ['1325852']
}

# Mappings from a monorail label to a buganizer hotlist, if any.
LABEL_MAP_CR2B = {
  # Chromium
  'Type-Bug-Regression': '5438261',
  'Chromeperf-Auto-NeedsAttention': '5670048',

  # Fuchsia
  'Performance': '5424295', # Performance

  # Test only. Remove after migration.
  'chromeperf-test': '5141966',
  'chromeperf-test-2': '5142065'
}

# Mappings from a buganizer hotlist to a monorail label.
HOTLIST_MAP_B2CR = {
  hotlist:label for label, hotlist in LABEL_MAP_CR2B.items()
}


def FindBuganizerComponentId(monorail_component):
  return COMPONENT_MAP_CR2B.get(monorail_component, 1325852)


def FindBuganizerComponents(monorail_project_name):
  """return a list of components in buganizer based on the monorail project

  The current implementation is ad hoc as the component mappings are not
  fully set up on buganizer yet.
  """
  return PROJECT_MAP_CR2B.get(monorail_project_name, [])


def FindMonorailProject(buganizer_component_id):
  return COMPONENT_MAP_B2CR.get(buganizer_component_id, '')


def FindBuganizerHotlists(monorail_labels):
  '''Find the hotlist mappings for monorail labels.

  Some of the Monorail labels are mapped to Buganizer hotlists. However,
  some of them are not, and they will be copied over to a custome field
  in Buganizer. For Fuchsia project, the custome field is 'Monorail labels',
  for other project, the custome field is 'Chromium labels'.
  Args:
    monorail_labels: the labels in Monorail
  Returns:
    hotlists: the hotlists in Buganizer
    extra_labels: the Monorail labels with no mapping in Buganizer
  '''
  hotlists = []
  extra_labels = []
  for label in monorail_labels:
    hotlist = _FindBuganizerHotlist(label)
    if hotlist:
      hotlists.append(hotlist)
    else:
      extra_labels.append(label)
  logging.debug(
    '[PerfIssueService] labels (%s) -> hotlists (%s). Leftover: %s',
    monorail_labels, hotlists, extra_labels)
  return hotlists, extra_labels


def _FindBuganizerHotlist(monorail_label):
  return LABEL_MAP_CR2B.get(monorail_label, None)


def _FindMonorailLabel(buganizer_hotlist):
  return HOTLIST_MAP_B2CR.get(buganizer_hotlist, None)


def _FindMonorailStatus(buganizer_status):
  if buganizer_status == 'NEW':
    return 'Unconfirmed'
  elif buganizer_status == 'ASSIGNED':
    return "Assigned"
  elif buganizer_status == 'ACCEPTED':
    return 'Started'
  elif buganizer_status == 'FIXED':
    return 'Fixed'
  elif buganizer_status == 'VERIFIED':
    return 'Verified'
  elif buganizer_status == 'DUPLICATE':
    return 'Duplicate'
  elif buganizer_status == 'OBSOLETE':
    return 'WontFix'
  return 'Untriaged'


def FindBuganizerStatus(monorail_status):
  if monorail_status in ('Unconfirmed', 'Untriaged', 'Available'):
    return 'NEW'
  elif monorail_status == 'Assigned':
    return "ASSIGNED"
  elif monorail_status == 'Started':
    return 'ACCEPTED'
  elif monorail_status == 'Fixed':
    return 'FIXED'
  elif monorail_status == 'Verified':
    return 'VERIFIED'
  elif monorail_status == 'WontFix':
    return 'OBSOLETE'
  elif monorail_status == 'Duplicate':
    return 'DUPLICATE'
  return 'STATUS_UNSPECIFIED'

# ============ mapping helpers end ============

def GetBuganizerStatusUpdate(issue_update, status_enum):
  ''' Get the status from an issue update

  In Buganizer, each update event is saved in an issueUpdate struct like:
    {
      "author": {
        "emailAddress": "wenbinzhang@google.com"
      },
      "timestamp": "2023-07-28T22: 17: 34.721Z",
      ...
      "fieldUpdates": [
        {
          "field": "status",
          "singleValueUpdate": {
            "oldValue": {
              "@type": "type.googleapis.com/google.protobuf.Int32Value",
              "value": 2
            },
            "newValue": {
              "@type": "type.googleapis.com/google.protobuf.Int32Value",
              "value": 4
            }
          }
        }
      ],
      ...
      "version": 5,
      "issueId": "285172796"
    }
  Noticed that each update on the UI can have multiple operations, but each of
  operations is a single issue update. So far we only use the 'status' value,
  thus the other updates like priority change are not returned here.
  '''
  for field_update in issue_update.get('fieldUpdates', []):
    if field_update.get('field') == 'status':
      new_status = field_update.get('singleValueUpdate').get('newValue').get('value')

      # The current status returned from issueUpdate is integer, which is
      # different from the definition in the discovery doc.
      # I'm using  service._schema.get() to load the schema in JSON.
      if isinstance(new_status, int):
        new_status = status_enum[new_status]

      status_update = {
        'status': _FindMonorailStatus(new_status)
      }
      return status_update
  return None


def LoadPriorityFromMonorailLabels(monorail_labels):
  ''' Load the priority from monorail labels if it is set

  In Monorail, a label like 'Pri-X' is used set the issue's priority to X.
  X can be range from 0 to 4. Currently Monorail scan the labels in order
  and set the priority as it sees any. Thus, the last X will be kept. Here
  we changed the logic to keep the highest level (lowest value.)

  Args:
    monorail_labels: a list of labels in monorail fashion.

  Returns:
    the priority from the label if any, otherwise 2 by default.
  '''
  if monorail_labels:
    for label in monorail_labels:
      if label.startswith('Pri-'):
        label_priority = int(label[4])
        if 0 <= label_priority < 5:
          return label_priority
  return 2

def ReconcileBuganizerIssue(buganizer_issue):
  '''Reconcile a Buganizer issue into the Monorail format

  During the Buganizer migration, we try to minimize the changes on the
  consumers of the issues. Thus, we will reconcile the results into the
  exising Monorail format before returning the results.
  '''
  monorail_issue = {}
  issue_state = buganizer_issue.get('issueState')

  project_id = FindMonorailProject(issue_state['componentId'])
  monorail_issue['projectId'] = project_id

  monorail_issue['id'] = buganizer_issue['issueId']

  buganizer_status = issue_state['status']
  if buganizer_status in ('NEW', 'ASSIGNED', 'ACCEPTED'):
    monorail_issue['state'] = 'open'
  else:
    monorail_issue['state'] = 'closed'

  monorail_issue['status'] = _FindMonorailStatus(buganizer_status)

  monorail_author = {
    'name': issue_state['reporter']
  }
  monorail_issue['author'] = monorail_author

  monorail_issue['summary'] = issue_state['title']

  monorail_issue['owner'] = issue_state.get('assignee', None)

  hotlist_ids = buganizer_issue.get('hotlistIds', [])
  label_names = [_FindMonorailLabel(hotlist_id) for hotlist_id in hotlist_ids]
  custom_field_id = GetCustomFieldId(project_id)
  all_custom_fields = issue_state.get('customFields', [])
  custom_labels = []
  for custom_field in all_custom_fields:
    if custom_field['customFieldId'] == str(custom_field_id):
      custom_labels = custom_field['repeatedTextValue'].get('values', [])
  label_names = list(set(label_names) | set(custom_labels))
  monorail_issue['labels'] = [label for label in label_names if label]

  return monorail_issue


def FindBuganizerIdByMonorailId(monorail_project, monorail_id):
  '''Try to find the buganizer id using the monorail id

  After a monorail issue is migrated to buganizer, the buganizer id will
  be populated to the monorail issue record, in a property 'migratedId'.
  '''
  logging.debug('Looking for b/ id for crbug %s in %s', monorail_id, monorail_project)
  if int(monorail_id) < 2000000:
    # This is a hack to handle the use case that:
    #  - we have the monorail issue id in our database
    #  - the issue is migrated to buganizer
    #  - we need to update the issue but we don't know the id on buganizer
    # Assuming all monorail id are less than 2000000, trying to access an
    # issue using buganizer client and a monorail id means the project has
    # been migrated.
    # In this case, we should find the buganizer id first.

    client = monorail_client.MonorailClient()
    issue = client.GetIssue(
      issue_id=monorail_id,
      project=monorail_project)
    buganizer_id = issue.get('migrated_id', None)
    logging.debug('Migrated ID %s found for %s/%s.',
                  buganizer_id, monorail_project, monorail_id)
    if not buganizer_id:
      err_msg = 'Cannot find the migrated id for crbug %s in %s' % (
        monorail_id, monorail_project)
      logging.error(err_msg)
    return buganizer_id
  return monorail_id


def GetCustomField(monorail_project):
  '''Get the custom field name based on monorail project.

  More context in FindBuganizerHotlists()
  '''
  if monorail_project == 'fuchsia':
     return 'customfield1241047'
  elif monorail_project == 'webrtc':
    return 'customfield1308691'
  return 'customfield1223031'


def GetCustomFieldId(monorail_project):
  '''Get the custom field ID based on monorail project.

  More context in FindBuganizerHotlists()
  '''
  if monorail_project == 'fuchsia':
     return 1241047
  elif monorail_project == 'webrtc':
    return 1308691
  return 1223031
