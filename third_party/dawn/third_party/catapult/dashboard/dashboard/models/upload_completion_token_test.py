# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import time
import unittest
import uuid

from dashboard.common import testing_common
from dashboard.models import upload_completion_token


class UploadCompletionTokenTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    testing_common.SetIsInternalUser('foo@bar.com', True)
    self.SetCurrentUser('foo@bar.com', is_admin=True)

  def testPutAndUpdate(self):
    token_id = str(uuid.uuid4())
    gcs_file = 'path/%s.gcs' % str(uuid.uuid4())
    token = upload_completion_token.Token(
        id=token_id, temporary_staging_file_path=gcs_file).put().get()

    self.assertEqual(token_id, token.key.id())
    self.assertEqual(gcs_file, token.temporary_staging_file_path)
    self.assertEqual(token.state, upload_completion_token.State.PENDING)

    # Sleep for 1 second, so update_time change is visible.
    sleep_time = datetime.timedelta(seconds=1)
    time.sleep(sleep_time.total_seconds())

    new_state = upload_completion_token.State.PROCESSING
    token.UpdateState(new_state)

    changed_token = upload_completion_token.Token.get_by_id(token_id)
    self.assertEqual(token_id, changed_token.key.id())
    self.assertEqual(gcs_file, changed_token.temporary_staging_file_path)
    self.assertTrue(
        (changed_token.update_time - changed_token.creation_time) >= sleep_time)
    self.assertEqual(changed_token.state, new_state)

    new_state = upload_completion_token.State.COMPLETED
    changed_token.UpdateState(new_state)

    changed_token = upload_completion_token.Token.get_by_id(token_id)
    self.assertEqual(changed_token.state, new_state)

  def testStatusUpdateWithMeasurements(self):
    token = upload_completion_token.Token(id=str(uuid.uuid4())).put().get()
    self.assertEqual(token.state, upload_completion_token.State.PENDING)
    measurement1 = token.AddMeasurement('test/1', True).get_result()
    measurement2 = token.AddMeasurement('test/2', False).get_result()

    self.assertEqual(token.state, upload_completion_token.State.PROCESSING)

    token.UpdateState(upload_completion_token.State.PROCESSING)
    self.assertEqual(token.state, upload_completion_token.State.PROCESSING)

    measurement1.state = upload_completion_token.State.FAILED
    measurement1.put()
    self.assertEqual(token.state, upload_completion_token.State.FAILED)

    token.UpdateState(upload_completion_token.State.COMPLETED)
    measurement2.state = upload_completion_token.State.COMPLETED
    measurement2.put()
    self.assertEqual(token.state, upload_completion_token.State.FAILED)

    measurement1.state = upload_completion_token.State.COMPLETED
    measurement1.put()
    self.assertEqual(token.state, upload_completion_token.State.COMPLETED)

  def testStatusUpdateWithExpiredMeasurement(self):
    token = upload_completion_token.Token(id=str(uuid.uuid4())).put().get()
    measurement1 = token.AddMeasurement('test/1', True).get_result()
    measurement2 = token.AddMeasurement('test/2', False).get_result()

    measurement1.key.delete()

    self.assertEqual(token.state, upload_completion_token.State.PROCESSING)

    token.UpdateState(upload_completion_token.State.COMPLETED)
    self.assertEqual(token.state, upload_completion_token.State.PROCESSING)

    measurement2.state = upload_completion_token.State.COMPLETED
    measurement2.put()
    self.assertEqual(token.state, upload_completion_token.State.COMPLETED)

  def testCreateSameMeasurementsForDifferentTokens(self):
    test_path = 'test/path'
    token1 = upload_completion_token.Token(id=str(uuid.uuid4())).put().get()
    token1.AddMeasurement(test_path, True).wait()

    token2 = upload_completion_token.Token(id=str(uuid.uuid4())).put().get()
    token2.AddMeasurement(test_path, True).wait()

    measurement1 = upload_completion_token.Measurement.GetByPath(
        test_path, token1.key.id())
    measurement2 = upload_completion_token.Measurement.GetByPath(
        test_path, token2.key.id())

    self.assertNotEqual(measurement1, measurement2)

  def testUpdateObjectState(self):
    target_state = upload_completion_token.State.COMPLETED

    upload_completion_token.Token.UpdateObjectState(None, target_state)

    token = upload_completion_token.Token(id=str(uuid.uuid4())).put().get()
    upload_completion_token.Token.UpdateObjectState(token, target_state)
    self.assertEqual(token.state, target_state)

  def testUpdateObjectState_ErrorMessage(self):
    target_state = upload_completion_token.State.FAILED
    target_message = 'Some message'

    token = upload_completion_token.Token(id=str(uuid.uuid4())).put().get()
    upload_completion_token.Token.UpdateObjectState(token, target_state,
                                                    target_message)
    self.assertEqual(token.state, target_state)
    self.assertEqual(token.error_message, target_message)

  @unittest.expectedFailure
  def testUpdateObjectStateWith_ErrorMessageFail(self):
    token = upload_completion_token.Token(id=str(uuid.uuid4())).put().get()
    upload_completion_token.Token.UpdateObjectState(
        token, upload_completion_token.State.FAILED, 'Some message')

  def testMeasurementUpdateStateByPathAsync(self):
    test_path = 'test/path'
    token_id = str(uuid.uuid4())
    target_state = upload_completion_token.State.COMPLETED
    token_key = upload_completion_token.Token(id=token_id).put()
    measurement_id = str(uuid.uuid4())
    upload_completion_token.Measurement(
        id=measurement_id, test_path=test_path, token=token_key).put()

    upload_completion_token.Measurement.UpdateStateByPathAsync(
        None, token_id, target_state).wait()
    upload_completion_token.Measurement.UpdateStateByPathAsync(
        test_path, None, target_state).wait()
    upload_completion_token.Measurement.UpdateStateByPathAsync(
        'expired', token_id, target_state).wait()

    measurement = upload_completion_token.Measurement.get_by_id(measurement_id)
    self.assertNotEqual(measurement.state, target_state)
    self.assertEqual(measurement.error_message, None)

    upload_completion_token.Measurement.UpdateStateByPathAsync(
        test_path, token_id, target_state).wait()

    measurement = upload_completion_token.Measurement.get_by_id(measurement_id)
    self.assertEqual(measurement.state, target_state)
    self.assertEqual(measurement.error_message, None)

  def testMeasurementUpdateStateByPathAsync_ErrorMessage(self):
    test_path = 'test/path'
    token_id = str(uuid.uuid4())
    target_state = upload_completion_token.State.FAILED
    target_message = 'Some message'
    token_key = upload_completion_token.Token(id=token_id).put()
    measurement_id = str(uuid.uuid4())
    upload_completion_token.Measurement(
        id=measurement_id, test_path=test_path, token=token_key).put()

    upload_completion_token.Measurement.UpdateStateByPathAsync(
        test_path, token_id, target_state, target_message).wait()

    measurement = upload_completion_token.Measurement.get_by_id(measurement_id)
    self.assertEqual(measurement.state, target_state)
    self.assertEqual(measurement.error_message, target_message)

  @unittest.expectedFailure
  def testMeasurementUpdateStateByPathAsync_ErrorMessageFail(self):
    test_path = 'test/path'
    token_id = str(uuid.uuid4())
    token_key = upload_completion_token.Token(id=token_id).put()
    upload_completion_token.Measurement(
        id=str(uuid.uuid4()), test_path=test_path, token=token_key).put()

    upload_completion_token.Measurement.UpdateStateByPathAsync(
        test_path, token_id, upload_completion_token.State.COMPLETED,
        'Some message').check_success()


if __name__ == '__main__':
  unittest.main()
