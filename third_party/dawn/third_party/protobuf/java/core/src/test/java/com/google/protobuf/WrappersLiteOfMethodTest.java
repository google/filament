// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package com.google.protobuf;

import static com.google.common.truth.Truth.assertThat;

import com.google.protobuf.wrapperstest.WrappersTestProto.TopLevelMessage;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class WrappersLiteOfMethodTest {

  @Test
  public void testOf() throws Exception {
    TopLevelMessage.Builder builder = TopLevelMessage.newBuilder();
    builder.setFieldDouble(DoubleValue.of(2.333));
    builder.setFieldFloat(FloatValue.of(2.333f));
    builder.setFieldInt32(Int32Value.of(2333));
    builder.setFieldInt64(Int64Value.of(23333333333333L));
    builder.setFieldUint32(UInt32Value.of(2333));
    builder.setFieldUint64(UInt64Value.of(23333333333333L));
    builder.setFieldBool(BoolValue.of(true));
    builder.setFieldString(StringValue.of("23333"));
    builder.setFieldBytes(BytesValue.of(ByteString.wrap("233".getBytes(Internal.UTF_8))));

    TopLevelMessage message = builder.build();
    assertThat(message.getFieldDouble().getValue()).isEqualTo(2.333);
    assertThat(message.getFieldFloat().getValue()).isEqualTo(2.333F);
    assertThat(message.getFieldInt32().getValue()).isEqualTo(2333);
    assertThat(message.getFieldInt64().getValue()).isEqualTo(23333333333333L);
    assertThat(message.getFieldUint32().getValue()).isEqualTo(2333);
    assertThat(message.getFieldUint64().getValue()).isEqualTo(23333333333333L);
    assertThat(true).isSameInstanceAs(message.getFieldBool().getValue());
    assertThat(message.getFieldString().getValue().equals("23333")).isTrue();
    assertThat(message.getFieldBytes().getValue().toStringUtf8().equals("233")).isTrue();
  }
}
