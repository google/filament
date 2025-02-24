# Copyright 2014 Dirk Pranke. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import unittest

from typ import test_case
from typ.host import Host
from typ.pool import make_pool, _MessageType, _ProcessPool, _loop


def _pre(host, worker_num, context):  # pylint: disable=W0613
    context['pre'] = True
    return context


def _post(context):
    context['post'] = True
    return context


def _echo(context, msg):
    return '%s/%s/%s' % (context['pre'], context['post'], msg)


def _error(context, msg):  # pylint: disable=W0613
    raise Exception('_error() raised Exception')


def _interrupt(context, msg):  # pylint: disable=W0613
    raise KeyboardInterrupt()


def _stub(*args):  # pylint: disable=W0613
    return None


class TestPool(test_case.TestCase):

    def run_basic_test(self, jobs):
        host = Host()
        context = {'pre': False, 'post': False}
        pool = make_pool(host, jobs, False, _echo, context, _pre, _post)
        pool.send('hello')
        pool.send('world')
        msg1 = pool.get()
        msg2 = pool.get()
        pool.close()
        final_contexts = pool.join()
        self.assertEqual(set([msg1, msg2]),
                         set(['True/False/hello',
                              'True/False/world']))
        expected_context = {'pre': True, 'post': True}
        expected_final_contexts = [expected_context for _ in range(jobs)]
        self.assertEqual(final_contexts, expected_final_contexts)

    def run_through_loop(self, callback=None, pool=None):
        callback = callback or _stub
        if pool:
            host = pool.host
        else:
            host = Host()
            pool = _ProcessPool(host, 0, False, _stub, None, _stub, _stub)
            pool.send('hello')

        worker_num = 1
        _loop(pool.request_pool, pool.responses, host, worker_num, callback,
              None, _stub, _stub, should_loop=False)
        return pool

    def test_async_close(self):
        host = Host()
        pool = make_pool(host, 1, False, _echo, None, _stub, _stub)
        pool.join()

    def test_basic_one_job(self):
        self.run_basic_test(1)

    def test_basic_two_jobs(self):
        self.run_basic_test(2)

    def test_join_discards_messages(self):
        host = Host()
        context = {'pre': False, 'post': False}
        pool = make_pool(host, 2, False, _echo, context, _pre, _post)
        pool.send('hello')
        pool.close()
        pool.join()
        self.assertEqual(len(pool.discarded_responses), 1)

    @unittest.skipIf(sys.version_info.major == 3, 'fails under python3')
    def test_join_gets_an_error(self):
        host = Host()
        pool = make_pool(host, 2, False, _error, None, _stub, _stub)
        pool.send('hello')
        pool.close()
        try:
            pool.join()
        except Exception as e:
            self.assertIn('_error() raised Exception', str(e))

    def test_join_gets_an_interrupt(self):
        host = Host()
        pool = make_pool(host, 2, False, _interrupt, None, _stub, _stub)
        pool.send('hello')
        pool.close()
        self.assertRaises(KeyboardInterrupt, pool.join)

    def test_loop(self):
        pool = self.run_through_loop()
        resp = pool.get()
        self.assertEqual(resp, None)
        pool.request_pool.put((_MessageType.Close, None))
        pool.close()
        self.run_through_loop(pool=pool)
        pool.join()

    def test_loop_fails_to_respond(self):
        # This tests what happens if _loop() tries to send a response
        # on a closed queue; we can't simulate this directly through the
        # api in a single thread.
        pool = self.run_through_loop()
        pool.request_pool.put((_MessageType.Request, None))
        pool.request_pool.put((_MessageType.Close, None))
        self.run_through_loop(pool=pool)
        pool.join()

    @unittest.skipIf(sys.version_info.major == 3, 'fails under python3')
    def test_loop_get_raises_error(self):
        pool = self.run_through_loop(_error)
        self.assertRaises(Exception, pool.get)
        pool.request_pool.put((_MessageType.Close, None))
        pool.close()
        pool.join()

    def test_loop_get_raises_interrupt(self):
        pool = self.run_through_loop(_interrupt)
        self.assertRaises(KeyboardInterrupt, pool.get)
        pool.request_pool.put((_MessageType.Close, None))
        pool.close()
        pool.join()

    def test_pickling_errors(self):
        def unpicklable_fn():  # pragma: no cover
            pass

        host = Host()
        jobs = 2
        self.assertRaises(Exception, make_pool,
                          host, jobs, False, _stub, unpicklable_fn, None, None)
        self.assertRaises(Exception, make_pool,
                          host, jobs, False, _stub, None, unpicklable_fn, None)
        self.assertRaises(Exception, make_pool,
                          host, jobs, False, _stub, None, None, unpicklable_fn)

    def test_no_close(self):
        host = Host()
        context = {'pre': False, 'post': False}
        pool = make_pool(host, 2, False, _echo, context, _pre, _post)
        final_contexts = pool.join()
        self.assertEqual(final_contexts, [])
