# Copyright 2019 Google Inc. All rights reserved.
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


def bytes_to_str(msg):
  return msg.decode('utf-8') if isinstance(msg, bytes) else msg


def str_to_bytes(msg):
  return msg.encode() if isinstance(msg, str) else msg


def is_str(msg):
  return (isinstance(msg, str) or
          sys.version_info.major == 2 and isinstance(msg, unicode))
