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

import protobuf_unittest.UnittestProto.TestAllExtensions;
import protobuf_unittest.UnittestProto.TestAllTypes;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** Unit test for {@link LazyField}. */
@RunWith(JUnit4.class)
public class LazyFieldTest {

  @Test
  public void testHashCode() {
    MessageLite message = TestUtil.getAllSet();
    LazyField lazyField = createLazyFieldFromMessage(message);
    assertThat(message.hashCode()).isEqualTo(lazyField.hashCode());
    lazyField.getValue();
    assertThat(message.hashCode()).isEqualTo(lazyField.hashCode());
    changeValue(lazyField);
    // make sure two messages have different hash code
    assertNotEqual(message.hashCode(), lazyField.hashCode());
  }

  @Test
  public void testHashCodeEx() throws Exception {
    TestAllExtensions message = TestUtil.getAllExtensionsSet();
    LazyField lazyField = createLazyFieldFromMessage(message);
    assertThat(message.hashCode()).isEqualTo(lazyField.hashCode());
    lazyField.getValue();
    assertThat(message.hashCode()).isEqualTo(lazyField.hashCode());
    changeValue(lazyField);
    // make sure two messages have different hash code
    assertNotEqual(message.hashCode(), lazyField.hashCode());
  }

  @Test
  public void testGetValue() {
    MessageLite message = TestUtil.getAllSet();
    LazyField lazyField = createLazyFieldFromMessage(message);
    assertThat(message).isEqualTo(lazyField.getValue());
    changeValue(lazyField);
    assertNotEqual(message, lazyField.getValue());
  }

  @Test
  public void testGetValueEx() throws Exception {
    TestAllExtensions message = TestUtil.getAllExtensionsSet();
    LazyField lazyField = createLazyFieldFromMessage(message);
    assertThat(message).isEqualTo(lazyField.getValue());
    changeValue(lazyField);
    assertNotEqual(message, lazyField.getValue());
  }

  @Test
  public void testEqualsObject() {
    MessageLite message = TestUtil.getAllSet();
    LazyField lazyField = createLazyFieldFromMessage(message);
    assertThat(lazyField).isEqualTo(message);
    changeValue(lazyField);
    assertThat(lazyField).isNotEqualTo(message);
    assertThat(message).isNotEqualTo(lazyField.getValue());
  }

  @Test
  @SuppressWarnings("TruthIncompatibleType") // LazyField.equals() is not symmetric
  public void testEqualsObjectEx() throws Exception {
    TestAllExtensions message = TestUtil.getAllExtensionsSet();
    LazyField lazyField = createLazyFieldFromMessage(message);
    assertThat(lazyField).isEqualTo(message);
    changeValue(lazyField);
    assertThat(lazyField).isNotEqualTo(message);
    assertThat(message).isNotEqualTo(lazyField.getValue());
  }

  // Help methods.

  private LazyField createLazyFieldFromMessage(MessageLite message) {
    ByteString bytes = message.toByteString();
    return new LazyField(
        message.getDefaultInstanceForType(), TestUtil.getExtensionRegistry(), bytes);
  }

  private void changeValue(LazyField lazyField) {
    TestAllTypes.Builder builder = TestUtil.getAllSet().toBuilder();
    builder.addRepeatedBool(true);
    MessageLite newMessage = builder.build();
    lazyField.setValue(newMessage);
  }

  private void assertNotEqual(Object unexpected, Object actual) {
    assertThat(unexpected).isNotSameInstanceAs(actual);
    assertThat((unexpected != null && unexpected.equals(actual))).isFalse();
  }
}
