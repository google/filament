# Copyright 2014 Google Inc. All rights reserved.
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

import copy
import multiprocessing
import pickle
import traceback

from typ.host import Host


def make_pool(host, jobs, stable_jobs, callback, context, pre_fn, post_fn):
    _validate_args(context, pre_fn, post_fn)
    if jobs > 1:
        return _ProcessPool(host, jobs, stable_jobs, callback, context, pre_fn, post_fn)
    else:
        return _AsyncPool(host, jobs, callback, context, pre_fn, post_fn)


class _MessageType(object):
    Request = 'Request'
    Response = 'Response'
    Close = 'Close'
    Done = 'Done'
    Error = 'Error'
    Interrupt = 'Interrupt'

    values = [Request, Response, Close, Done, Error, Interrupt]


def _validate_args(context, pre_fn, post_fn):
    try:
        _ = pickle.dumps(context)
    except Exception as e:
        raise ValueError('context passed to make_pool is not picklable: %s'
                         % str(e))
    try:
        _ = pickle.dumps(pre_fn)
    except pickle.PickleError:
        raise ValueError('pre_fn passed to make_pool is not picklable')
    try:
        _ = pickle.dumps(post_fn)
    except pickle.PickleError:
        raise ValueError('post_fn passed to make_pool is not picklable')


class _RequestPool(object):
    def __init__(self, jobs, stable_jobs):
        self.next_request_index = 0
        self.stable_jobs = stable_jobs
        num_queues = jobs if stable_jobs else 1
        self.requests = [multiprocessing.Queue() for _ in range(num_queues)]

    def put(self, request):
        self.get_request_queue(self.next_request_index).put(request)
        self.next_request_index = (self.next_request_index + 1) % len(self.requests)

    def get_request_queue(self, job):
        return self.requests[job] if self.stable_jobs else self.requests[0]


class _ProcessPool(object):

    def __init__(self, host, jobs, stable_jobs, callback, context, pre_fn, post_fn):
        self.host = host
        self.jobs = jobs
        self.request_pool = _RequestPool(jobs, stable_jobs)
        self.responses = multiprocessing.Queue()
        self.workers = []
        self.discarded_responses = []
        self.closed = False
        self.erred = False
        for worker_num in range(1, jobs + 1):
            w = multiprocessing.Process(target=_loop,
                                        args=(self.request_pool,
                                              self.responses, host.for_mp(),
                                              worker_num, callback, context,
                                              pre_fn, post_fn))
            w.start()
            self.workers.append(w)

    def send(self, msg, msg_type=_MessageType.Request):
        self.request_pool.put((_MessageType.Request, msg))

    def get(self):
        msg_type, resp = self.responses.get()
        if msg_type == _MessageType.Error:
            self._handle_error(resp)
        elif msg_type == _MessageType.Interrupt:
            raise KeyboardInterrupt
        assert msg_type == _MessageType.Response
        return resp

    def close(self):
        for _ in self.workers:
            self.request_pool.put((_MessageType.Close, None))
        self.closed = True

    def join(self):
        # TODO: one would think that we could close self.requests in close(),
        # above, and close self.responses below, but if we do, we get
        # weird tracebacks in the daemon threads multiprocessing starts up.
        # Instead, we have to hack the innards of multiprocessing. It
        # seems likely that there's a bug somewhere, either in this module or
        # in multiprocessing.
        # pylint: disable=protected-access
        if self.host.is_python3:  # pragma: python3
            multiprocessing.queues.is_exiting = lambda: True
        else:  # pragma: python2
            multiprocessing.util._exiting = True

        if not self.closed:
            # We must be aborting; terminate the workers rather than
            # shutting down cleanly.
            for w in self.workers:
                w.terminate()
                w.join()
            return []

        final_responses = []
        error = None
        interrupted = None
        for w in self.workers:
            while True:
                msg_type, resp = self.responses.get()
                if msg_type == _MessageType.Error:
                    error = resp
                    break
                if msg_type == _MessageType.Interrupt:
                    interrupted = True
                    break
                if msg_type == _MessageType.Done:
                    final_responses.append(resp[1])
                    break
                self.discarded_responses.append(resp)

        for w in self.workers:
            w.join()

        # TODO: See comment above at the beginning of the function for
        # why this is commented out.
        # self.responses.close()

        if error:
            self._handle_error(error)
        if interrupted:
            raise KeyboardInterrupt
        return final_responses

    def _handle_error(self, msg):
        worker_num, tb = msg
        self.erred = True
        raise Exception("Error from worker %d (traceback follows):\n%s" %
                        (worker_num, tb))


# 'Too many arguments' pylint: disable=R0913

def _loop(request_pool, responses, host, worker_num,
          callback, context, pre_fn, post_fn, should_loop=True):
    requests = request_pool.get_request_queue(worker_num - 1)
    host = host or Host()
    try:
        context_after_pre = pre_fn(host, worker_num, context)
        keep_looping = True
        while keep_looping:
            message_type, args = requests.get()
            if message_type == _MessageType.Close:
                responses.put((_MessageType.Done,
                               (worker_num, post_fn(context_after_pre))))
                # crbug.com/1298810: Need to give the queue's I/O thread time
                # to put the done message into the underlying OS pipe. Without
                # this, the parent process's end of the queue may have the
                # updated `qsize` but will never receive the message. See also:
                # https://docs.python.org/3/library/multiprocessing.html#multiprocessing.Queue.join_thread
                responses.close()
                responses.join_thread()
                break
            assert message_type == _MessageType.Request
            resp = callback(context_after_pre, args)
            responses.put((_MessageType.Response, resp))
            keep_looping = should_loop
    except KeyboardInterrupt as e:
        responses.put((_MessageType.Interrupt, (worker_num, str(e))))
    except Exception:
        responses.put((_MessageType.Error,
                       (worker_num, traceback.format_exc())))


class _AsyncPool(object):

    def __init__(self, host, jobs, callback, context, pre_fn, post_fn):
        self.host = host or Host()
        self.jobs = jobs
        self.callback = callback
        self.context = copy.deepcopy(context)
        self.msgs = []
        self.closed = False
        self.post_fn = post_fn
        self.context_after_pre = pre_fn(self.host, 1, self.context)
        self.final_context = None

    def send(self, msg):
        self.msgs.append(msg)

    def get(self):
        return self.callback(self.context_after_pre, self.msgs.pop(0))

    def close(self):
        self.closed = True
        self.final_context = self.post_fn(self.context_after_pre)

    def join(self):
        if not self.closed:
            self.close()
        return [self.final_context]


class _NoOpPool(object):
    def close(self):
        pass

    def join(self):
        return []


class _PoolGroup(object):
    """Abstraction layer for global vs. scoped pools.

    Most tests should be able to run fine with scoped pools, but provide the
    older global pool behavior as an option in case it's needed.

    If using scoped pools, the global pool is a _NoOpPool and all work is done
    on the scoped pools.

    If using global pools, the scoped pools map to the global pool.
    """
    def __init__(self, host, jobs, stable_jobs, callback, context, pre_fn, post_fn):
        self.host = host
        self.jobs = jobs
        self.stable_jobs = stable_jobs
        self.callback = callback
        self.context = context
        self.pre_fn = pre_fn
        self.post_fn = post_fn

        self.global_pool = None
        self.parallel_pool = None
        self.serial_pool = None

    def make_global_pool(self):
        raise NotImplementedError()

    def close_global_pool(self):
        self.global_pool.close()

    def join_global_pool(self):
        return self.global_pool.join()

    def make_parallel_pool(self):
        raise NotImplementedError()

    def close_parallel_pool(self):
        raise NotImplementedError()

    def join_parallel_pool(self):
        raise NotImplementedError()

    def make_serial_pool(self):
        raise NotImplementedError()

    def close_serial_pool(self):
        raise NotImplementedError()

    def join_serial_pool(self):
        raise NotImplementedError()


class _GlobalPoolGroup(_PoolGroup):
    def make_global_pool(self):
        assert self.global_pool is None
        self.global_pool = make_pool(self.host, self.jobs, self.stable_jobs,
                                     self.callback, self.context,
                                     self.pre_fn, self.post_fn)
        return self.global_pool

    def make_parallel_pool(self):
        assert self.global_pool
        self.parallel_pool = self.global_pool
        return self.parallel_pool

    def close_parallel_pool(self):
        pass

    def join_parallel_pool(self):
        return []

    def make_serial_pool(self):
        assert self.global_pool
        self.serial_pool = self.global_pool
        return self.serial_pool

    def close_serial_pool(self):
        pass

    def join_serial_pool(self):
        return []


class _ScopedPoolGroup(_PoolGroup):
    def make_global_pool(self):
        assert self.global_pool is None
        self.global_pool = _NoOpPool()
        return self.global_pool

    def make_parallel_pool(self):
        if self.parallel_pool:
            assert self.parallel_pool.closed
        self.parallel_pool = make_pool(self.host, self.jobs, self.stable_jobs,
                                       self.callback, self.context,
                                       self.pre_fn, self.post_fn)
        return self.parallel_pool

    def close_parallel_pool(self):
        self.parallel_pool.close()

    def join_parallel_pool(self):
        return self.parallel_pool.join()

    def make_serial_pool(self):
        if self.serial_pool:
            assert self.serial_pool.closed
        self.serial_pool = make_pool(self.host, 1, self.stable_jobs, self.callback,
                                     self.context, self.pre_fn,
                                     self.post_fn)
        return self.serial_pool

    def close_serial_pool(self):
        self.serial_pool.close()

    def join_serial_pool(self):
        return self.serial_pool.join()


def make_pool_group(host, jobs, stable_jobs, callback, context, pre_fn, post_fn, use_global):
    if use_global:
        return _GlobalPoolGroup(host, jobs, stable_jobs, callback, context, pre_fn, post_fn)
    return _ScopedPoolGroup(host, jobs, stable_jobs, callback, context, pre_fn, post_fn)
