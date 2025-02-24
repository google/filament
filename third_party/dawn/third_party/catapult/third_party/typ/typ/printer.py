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


class Printer(object):

    def __init__(self, print_, should_overwrite, cols):
        self.print_ = print_
        self.should_overwrite = should_overwrite
        self.cols = cols
        self.last_line = ''

    def flush(self):
        if self.last_line:
            self.print_('')
            self.last_line = ''

    def update(self, msg, elide=True):
        msg_len = len(msg)
        if elide and self.cols and msg_len > self.cols - 5:
            new_len = int((self.cols - 5) / 2)
            msg = msg[:new_len] + '...' + msg[-new_len:]
        if self.should_overwrite and self.last_line:
            self.print_('\r' + ' ' * len(self.last_line) + '\r', end='')
        elif self.last_line:
            self.print_('')
        self.print_(msg, end='')
        last_nl = msg.rfind('\n')
        self.last_line = msg[last_nl + 1:]
