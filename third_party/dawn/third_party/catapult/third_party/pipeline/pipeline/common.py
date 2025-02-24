#!/usr/bin/env python
#
# Copyright 2010 Google Inc.
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

"""Common Pipelines for easy reuse."""

import cgi
import logging
import random

from google.appengine.api import mail
from google.appengine.api import taskqueue

import pipeline


class Return(pipeline.Pipeline):
  """Causes calling generator to have the supplied default output value.

  Only works when yielded last!
  """

  def run(self, return_value=None):
    return return_value


class Ignore(pipeline.Pipeline):
  """Mark the supplied parameters as unused outputs of sibling pipelines."""

  def run(self, *args):
    pass


class Dict(pipeline.Pipeline):
  """Returns a dictionary with the supplied keyword arguments."""

  def run(self, **kwargs):
    return dict(**kwargs)


class List(pipeline.Pipeline):
  """Returns a list with the supplied positional arguments."""

  def run(self, *args):
    return list(args)


class AbortIfTrue(pipeline.Pipeline):
  """Aborts the entire pipeline if the supplied argument is True."""

  def run(self, value, message=''):
    if value:
      raise pipeline.Abort(message)


class All(pipeline.Pipeline):
  """Returns True if all of the values are True.

  Returns False if there are no values present.
  """

  def run(self, *args):
    if len(args) == 0:
      return False
    for value in args:
      if not value:
        return False
    return True


class Any(pipeline.Pipeline):
  """Returns True if any of the values are True."""

  def run(self, *args):
    for value in args:
      if value:
        return True
    return False


class Complement(pipeline.Pipeline):
  """Returns the boolean complement of the values."""

  def run(self, *args):
    if len(args) == 1:
      return not args[0]
    else:
      return [not value for value in args]


class Max(pipeline.Pipeline):
  """Returns the max value."""

  def __init__(self, *args):
    if len(args) == 0:
      raise TypeError('max expected at least 1 argument, got 0')
    pipeline.Pipeline.__init__(self, *args)

  def run(self, *args):
    return max(args)


class Min(pipeline.Pipeline):
  """Returns the min value."""

  def __init__(self, *args):
    if len(args) == 0:
      raise TypeError('min expected at least 1 argument, got 0')
    pipeline.Pipeline.__init__(self, *args)

  def run(self, *args):
    return min(args)


class Sum(pipeline.Pipeline):
  """Returns the sum of all values."""

  def __init__(self, *args):
    if len(args) == 0:
      raise TypeError('sum expected at least 1 argument, got 0')
    pipeline.Pipeline.__init__(self, *args)

  def run(self, *args):
    return sum(args)


class Multiply(pipeline.Pipeline):
  """Returns all values multiplied together."""

  def __init__(self, *args):
    if len(args) == 0:
      raise TypeError('multiply expected at least 1 argument, got 0')
    pipeline.Pipeline.__init__(self, *args)

  def run(self, *args):
    total = 1
    for value in args:
      total *= value
    return total


class Negate(pipeline.Pipeline):
  """Returns each value supplied multiplied by -1."""

  def __init__(self, *args):
    if len(args) == 0:
      raise TypeError('negate expected at least 1 argument, got 0')
    pipeline.Pipeline.__init__(self, *args)

  def run(self, *args):
    if len(args) == 1:
      return -1 * args[0]
    else:
      return [-1 * x for x in args]


class Extend(pipeline.Pipeline):
  """Combine together lists and tuples into a single list.

  Args:
    *args: One or more lists or tuples.

  Returns:
    A single list of all supplied lists merged together in order. Length of
    the output list is the sum of the lengths of all input lists.
  """

  def run(self, *args):
    combined = []
    for value in args:
      combined.extend(value)
    return combined


class Append(pipeline.Pipeline):
  """Combine together values into a list.

  Args:
    *args: One or more values.

  Returns:
    A single list of all values appended to the same list. Length of the
    output list matches the length of the input list.
  """

  def run(self, *args):
    combined = []
    for value in args:
      combined.append(value)
    return combined


class Concat(pipeline.Pipeline):
  """Concatenates strings together using a join character.

  Args:
    *args: One or more strings.
    separator: Keyword argument only; the string to use to join the args.

  Returns:
    The joined string.
  """

  def run(self, *args, **kwargs):
    separator = kwargs.get('separator', '')
    return separator.join(args)


class Union(pipeline.Pipeline):
  """Like Extend, but the resulting list has all unique elements."""

  def run(self, *args):
    combined = set()
    for value in args:
      combined.update(value)
    return list(combined)


class Intersection(pipeline.Pipeline):
  """Returns only those items belonging to all of the supplied lists.

  Each argument must be a list. No individual items are permitted.
  """

  def run(self, *args):
    if not args:
      return []
    result = set(args[0])
    for value in args[1:]:
      result.intersection_update(set(value))
    return list(result)


class Uniquify(pipeline.Pipeline):
  """Returns a list of unique items from the list of items supplied."""

  def run(self, *args):
    return list(set(args))


class Format(pipeline.Pipeline):
  """Formats a string with formatting arguments."""

  @classmethod
  def dict(cls, message, **format_dict):
    """Formats a dictionary.

    Args:
      message: The format string.
      **format_dict: Keyword arguments of format parameters to use for
        formatting the string.

    Returns:
      The formatted string.
    """
    return cls('dict', message, format_dict)

  @classmethod
  def tuple(cls, message, *params):
    """Formats a tuple.

    Args:
      message: The format string.
      *params: The formatting positional parameters.

    Returns:
      The formatted string.
    """
    return cls('tuple', message, *params)

  def run(self, format_type, message, *params):
    if format_type == 'dict':
      return message % params[0]
    elif format_type == 'tuple':
      return message % params
    else:
      raise pipeline.Abort('Invalid format type: %s' % format_type)


class Log(pipeline.Pipeline):
  """Logs a message, just like the Python logging module."""

  # TODO: Hack the call stack of the logging message to use the file and line
  # context from when it was first scheduled, not when it actually ran.

  _log_method = logging.log

  @classmethod
  def log(cls, *args, **kwargs):
    return Log(*args, **kwargs)

  @classmethod
  def debug(cls, *args, **kwargs):
    return Log(logging.DEBUG, *args, **kwargs)

  @classmethod
  def info(cls, *args, **kwargs):
    return Log(logging.INFO, *args, **kwargs)

  @classmethod
  def warning(cls, *args, **kwargs):
    return Log(logging.WARNING, *args, **kwargs)

  @classmethod
  def error(cls, *args, **kwargs):
    return Log(logging.ERROR, *args, **kwargs)

  @classmethod
  def critical(cls, *args, **kwargs):
    return Log(logging.CRITICAL, *args, **kwargs)

  def run(self, level, message, *args):
    Log._log_method.im_func(level, message, *args)


class Delay(pipeline.Pipeline):
  """Waits N seconds before completion.

  Args:
    seconds: Keyword argument only. The number of seconds to wait. Will be
      rounded to the nearest whole second.

  Returns:
    How long this delay waited.
  """

  async = True

  def __init__(self, *args, **kwargs):
    if len(args) != 0 or len(kwargs) != 1 or kwargs.keys()[0] != 'seconds':
      raise TypeError('Delay takes one keyword parameter, "seconds".')
    pipeline.Pipeline.__init__(self, *args, **kwargs)

  def run(self, seconds=None):
    task = self.get_callback_task(
        countdown=seconds,
        name='ae-pipeline-delay-' + self.pipeline_id)
    try:
      task.add(self.queue_name)
    except (taskqueue.TombstonedTaskError, taskqueue.TaskAlreadyExistsError):
      pass

  def run_test(self, seconds=None):
    logging.debug('Delay pipeline pretending to sleep %0.2f seconds', seconds)
    self.complete(seconds)

  def callback(self):
    self.complete(self.kwargs['seconds'])


class EmailToContinue(pipeline.Pipeline):
  """Emails someone asking if the pipeline should continue.

  When the user clicks "Approve", the pipeline will return True. When the
  user clicks "Disapprove", the pipeline will return False.

  Supply normal mail.EmailMessage parameters, plus two additional parameters:

    approve_html: HTML to show to the user after clicking approve.
    disapprove_html: HTML to show to the user after clicking disapprove.

  Additionally, the 'body' and 'html' keyword arguments are treated as Python
  dictionary templates with the keywords 'approval_url' and 'disapprove_url',
  which let you place those links in your email however you want (as long
  as clicking the links results in a GET request). The approve/disapprove URLs
  are relative paths (e.g., '/relative/foo/bar'), so you must connect them to
  whatever hostname you actually want users to access the callback on with an
  absolute URL.

  A random token is used to secure the asynchronous action.
  """

  async = True
  public_callbacks = True

  _email_message = mail.EmailMessage

  def __init__(self, **kwargs):
    if 'random_token' not in kwargs:
      kwargs['random_token'] = '%x' % random.randint(0, 2**64)
    if 'approve_html' not in kwargs:
      kwargs['approve_html'] = '<h1>Approved!</h1>'
    if 'disapprove_html' not in kwargs:
      kwargs['disapprove_html'] = '<h1>Not Approved!</h1>'
    pipeline.Pipeline.__init__(self, **kwargs)

  def run(self, **kwargs):
    random_token = kwargs.pop('random_token')
    kwargs.pop('approve_html', '')
    kwargs.pop('disapprove_html', '')

    approve_url = self.get_callback_url(
        random_token=random_token, choice='approve')
    disapprove_url = self.get_callback_url(
        random_token=random_token, choice='disapprove')

    mail_args = kwargs.copy()
    mail_args['body'] = mail_args['body'] % {
        'approve_url': approve_url,
        'disapprove_url': disapprove_url,
    }
    if 'html' in mail_args:
      mail_args['html'] = mail_args['html'] % {
        'approve_url': cgi.escape(approve_url),
        'disapprove_url': cgi.escape(disapprove_url),
      }
    EmailToContinue._email_message.im_func(**mail_args).send()

  def run_test(self, **kwargs):
    self.run(**kwargs)
    self.complete(True)

  def callback(self, random_token=None, choice=None):
    if random_token != self.kwargs['random_token']:
      return (403, 'text/html', '<h1>Invalid security token.</h1>')

    if choice == 'approve':
      self.complete(True)
      return (200, 'text/html', self.kwargs['approve_html'])
    elif choice == 'disapprove':
      self.complete(False)
      return (200, 'text/html', self.kwargs['disapprove_html'])
    else:
      return (400, 'text/html', '<h1>Invalid "choice" value.</h1>')
