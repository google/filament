# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest
import validator


class ValidatorTest(unittest.TestCase):

  def testGoodCase(self):
    subscriptions = validator.Validate("""
          subscriptions: [{
            name: "Release Team"
            contact_email: "release-team@example.com"
            bug_labels: ["release-blocker"]
            bug_components: ["Sample>Component"]
              rules: { match: [{glob: "project/**"}] }
          },
          {
            name: "Memory Team",
            contact_email: "memory-team@example.com",
            bug_labels: ["memory-regressions"],
            rules: { match: [{regex: "^project/.*memory_.*$"}] },
            anomaly_configs: [
              {
                min_relative_change: 0.10
                rules: {
                  match: {
                    regex: "^project/platform/.*/memory_peak$"
                  }
                }
              }
            ]
          }]""")
    self.assertIsNotNone(subscriptions)

  def testMonorailProjectId(self):
    subscriptions = validator.Validate("""
          subscriptions: [{
            name: "Non-Chromium Team"
            contact_email: "release-team@example.com"
            bug_labels: ["release-blocker"]
            bug_components: ["Sample>Component"]
            monorail_project_id: "not-chromium"
            rules: { match: [{glob: "project/**"}] }
          },
          {
            name: "Memory Team",
            contact_email: "memory-team@example.com",
            bug_labels: ["memory-regressions"],
            rules: { match: [{regex: "^project/.*memory_.*$"}] },
            anomaly_configs: [
              {
                min_relative_change: 0.01
                rules: { match: {regex: "^project/platform/.*/memory_peak$"} }
              }
            ]
          }]""")
    self.assertIsNotNone(subscriptions)

  def testInvalidJSON(self):
    with self.assertRaisesRegex(validator.InvalidConfig,
                                'SheriffConfig Validation Error'):
      _ = validator.Validate("""
                             subscriptions: ...
                             """)

  def testMissingEmail(self):
    with self.assertRaises(validator.MissingEmail):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Missing Email",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: { match: [{glob: "project/**"}] }
                                 }
                               ]
                             """)

  def testMissingName(self):
    with self.assertRaises(validator.MissingName):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   contact_email: "missing-name@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: { match: [{glob: "project/**"}] }
                                 }
                               ]
                             """)

  def testMissingPattern(self):
    with self.assertRaises(validator.MissingPatterns):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "No Patterns",
                                   contact_email: "no-patterns@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"]
                                 }
                               ]
                             """)

  def testMissingEmptyPattern(self):
    with self.assertRaises(validator.MissingPatterns):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty List Patterns",
                                   contact_email: "no-patterns@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: { match: [] }
                                 }
                               ]
                             """)

  def testInvalidUndefinedPattern(self):
    with self.assertRaisesRegex(validator.InvalidPattern,
                                'must provide either \'glob\' or \'regex\''):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Bad Pattern",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: { match: [{}] }
                                 }
                               ]
                             """)

  def testInvalidEmptyGlob(self):
    with self.assertRaisesRegex(validator.InvalidPattern,
                                'glob must not be empty'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Glob",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: { match: [{glob: ""}] }
                                 }
                               ]
                             """)

  def testInvalidEmptyRegex(self):
    with self.assertRaisesRegex(validator.InvalidPattern,
                                'regex must not be empty'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: { match: [{regex: ""}] }
                                 }
                               ]
                             """)

  def testInvalidRegexMatch(self):
    with self.assertRaisesRegex(validator.InvalidPattern, 'bad pattern'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: { match: [{regex: "*"}] }
                                 }
                               ]
                             """)

  def testInvalidRegexExclude(self):
    with self.assertRaisesRegex(validator.InvalidPattern, 'bad pattern'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: {
                                      match: [{regex: ".*"}]
                                      exclude: [{regex: "*"}]
                                   }
                                 }
                               ]
                             """)

  def testInvalidRegexAutoTriage(self):
    with self.assertRaisesRegex(validator.InvalidPattern, 'bad pattern'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: {
                                      match: [{regex: ".*"}]
                                      exclude: [{regex: ".*"}]
                                   }
                                   auto_triage {
                                     rules: {
                                       match: {regex: "*"}
                                     }
                                   }
                                 }
                               ]
                             """)
    with self.assertRaisesRegex(validator.InvalidPattern, 'bad pattern'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: {
                                      match: [{regex: ".*"}]
                                      exclude: [{regex: ".*"}]
                                   }
                                   auto_triage {
                                     rules: {
                                       match: {regex: ".*"}
                                       exclude: {regex: "*"}
                                     }
                                   }
                                 }
                               ]
                             """)

  def testInvalidRegexAutoBisect(self):
    with self.assertRaisesRegex(validator.InvalidPattern, 'bad pattern'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: {
                                      match: [{regex: ".*"}]
                                   }
                                   auto_triage {
                                     rules: {
                                       match: {regex: ".*"}
                                     }
                                   }
                                   auto_bisection {
                                     rules: {
                                       match: {regex: "*"}
                                     }
                                   }
                                 }
                               ]
                             """)
    with self.assertRaisesRegex(validator.InvalidPattern, 'bad pattern'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: {
                                      match: [{regex: ".*"}]
                                      exclude: [{regex: ".*"}]
                                   }
                                   auto_triage {
                                     rules: {
                                       match: {regex: ".*"}
                                     }
                                   }
                                   auto_bisection {
                                     rules: {
                                       match: {regex: ".*"}
                                       exclude: {regex: "*"}
                                     }
                                   }
                                 }
                               ]
                             """)

  def testInvalidRegexAutoMerge(self):
    with self.assertRaisesRegex(validator.InvalidPattern, 'bad pattern'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: {
                                      match: [{regex: ".*"}]
                                   }
                                   auto_triage {
                                     rules: {
                                       match: {regex: ".*"}
                                     }
                                   }
                                   auto_merge {
                                     rules: {
                                       match: {regex: "*"}
                                     }
                                   }
                                 }
                               ]
                             """)
    with self.assertRaisesRegex(validator.InvalidPattern, 'bad pattern'):
      _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: {
                                      match: [{regex: ".*"}]
                                      exclude: [{regex: ".*"}]
                                   }
                                   auto_triage {
                                     rules: {
                                       match: {regex: ".*"}
                                     }
                                   }
                                   auto_merge {
                                     rules: {
                                       match: {regex: ".*"}
                                       exclude: {regex: "*"}
                                     }
                                   }
                                 }
                               ]
                             """)

  def testValidAutoTriageMergeAndBisect(self):
    _ = validator.Validate("""
                               subscriptions: [
                                 {
                                   name: "Empty Regex",
                                   contact_email: "bad-pattern@domain",
                                   bug_labels: ["test-blocker"],
                                   bug_components: ["Sample>Component"],
                                   rules: {
                                      match: [{regex: ".*"}]
                                      exclude: [{regex: ".*"}]
                                   }
                                   auto_triage {
                                     rules: {
                                       match: {regex: ".*"}
                                       exclude: {regex: ".*"}
                                     }
                                   }
                                   auto_merge {
                                     rules: {
                                       match: {regex: ".*"}
                                       exclude: {regex: ".*"}
                                     }
                                   }
                                   auto_bisection {
                                     rules: {
                                       match: {regex: ".*"}
                                       exclude: {regex: ".*"}
                                     }
                                   }
                                 }
                               ]
                             """)

  def testValidAnomalyConfigs(self):
    _ = validator.Validate("""
      subscriptions: [
        {
          name: "Subscription"
          contact_email: "user@domain"
          bug_labels: ["test-blocker"],
          bug_components: ["Sample>Component"],
          rules: {
            match: [{regex: ".*"}]
          }
          anomaly_configs {
            rules {
              match: [{regex: ".*"}]
            }
          }
        }
      ]""")

  def testInvalidAnomalyConfigs(self):
    pattern = """
        subscriptions: [
          {
            name: "Test %s"
            contact_email: "user@domain"
            bug_labels: ["test-blocker"]
            bug_components: ["Sample>Component"]
            rules {
              match {
                regex: ".*"
              }
            }
            anomaly_configs {
              max_window_size: 50
              min_segment_size: 6
              min_absolute_change: 0
              min_relative_change: 0.01
              min_steppiness: 0.5
              multiple_of_std_dev: 0
              rules {
                %s
              }
            }
          }
        ]"""
    cases = [
        'match { glob: "" }',
        'exclude { glob: "" }',
        'match { regex: "" }',
        'exclude { regex: "" }',
        'match { regex: "*" }',
        'exclude { regex: "*" }',
    ]
    for case, rule in enumerate(cases):
      with self.subTest(case=case, rule=rule):
        with self.assertRaisesRegex(validator.InvalidPattern, '.*'):
          _ = validator.Validate(pattern % (case, rule))


if __name__ == '__main__':
  unittest.main()
