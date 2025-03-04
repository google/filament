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
import static com.google.common.truth.Truth.assertWithMessage;

import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** Tests for {@link UnmodifiableLazyStringList}. */
@RunWith(JUnit4.class)
public class UnmodifiableLazyStringListTest {

  private static final String STRING_A = "A";
  private static final String STRING_B = "B";
  private static final String STRING_C = "C";

  private static final ByteString BYTE_STRING_A = ByteString.copyFromUtf8("A");
  private static final ByteString BYTE_STRING_B = ByteString.copyFromUtf8("B");
  private static final ByteString BYTE_STRING_C = ByteString.copyFromUtf8("C");

  @Test
  public void testReadOnlyMethods() {
    LazyStringArrayList rawList = createSampleList();
    UnmodifiableLazyStringList list = new UnmodifiableLazyStringList(rawList);
    assertThat(list).hasSize(3);
    assertThat(list.get(0)).isSameInstanceAs(STRING_A);
    assertThat(list.get(1)).isSameInstanceAs(STRING_B);
    assertThat(list.get(2)).isSameInstanceAs(STRING_C);
    assertThat(list.getByteString(0)).isEqualTo(BYTE_STRING_A);
    assertThat(list.getByteString(1)).isEqualTo(BYTE_STRING_B);
    assertThat(list.getByteString(2)).isEqualTo(BYTE_STRING_C);

    List<ByteString> byteStringList = list.asByteStringList();
    assertThat(byteStringList.get(0)).isSameInstanceAs(list.getByteString(0));
    assertThat(byteStringList.get(1)).isSameInstanceAs(list.getByteString(1));
    assertThat(byteStringList.get(2)).isSameInstanceAs(list.getByteString(2));
  }

  @Test
  public void testModifyMethods() {
    LazyStringArrayList rawList = createSampleList();
    UnmodifiableLazyStringList list = new UnmodifiableLazyStringList(rawList);

    try {
      list.remove(0);
      assertWithMessage("expected exception").fail();
    } catch (UnsupportedOperationException e) {
      // expected
    }
    assertThat(list).hasSize(3);

    try {
      list.add(STRING_B);
      assertWithMessage("expected exception").fail();
    } catch (UnsupportedOperationException e) {
      // expected
    }
    assertThat(list).hasSize(3);

    try {
      list.set(1, STRING_B);
      assertWithMessage("expected exception").fail();
    } catch (UnsupportedOperationException e) {
      // expected
    }
    assertThat(list).hasSize(3);

    List<ByteString> byteStringList = list.asByteStringList();
    try {
      byteStringList.remove(0);
      assertWithMessage("expected exception").fail();
    } catch (UnsupportedOperationException e) {
      // expected
    }
    assertThat(list).hasSize(3);
    assertThat(byteStringList).hasSize(3);

    try {
      byteStringList.add(BYTE_STRING_B);
      assertWithMessage("expected exception").fail();
    } catch (UnsupportedOperationException e) {
      // expected
    }
    assertThat(list).hasSize(3);
    assertThat(byteStringList).hasSize(3);

    try {
      byteStringList.set(1, BYTE_STRING_B);
      assertWithMessage("expected exception").fail();
    } catch (UnsupportedOperationException e) {
      // expected
    }
    assertThat(list).hasSize(3);
    assertThat(byteStringList).hasSize(3);
  }

  @Test
  public void testIterator() {
    LazyStringArrayList rawList = createSampleList();
    UnmodifiableLazyStringList list = new UnmodifiableLazyStringList(rawList);

    Iterator<String> iter = list.iterator();
    int count = 0;
    while (iter.hasNext()) {
      iter.next();
      count++;
      try {
        iter.remove();
        assertWithMessage("expected exception").fail();
      } catch (UnsupportedOperationException e) {
        // expected
      }
    }
    assertThat(count).isEqualTo(3);

    List<ByteString> byteStringList = list.asByteStringList();
    Iterator<ByteString> byteIter = byteStringList.iterator();
    count = 0;
    while (byteIter.hasNext()) {
      byteIter.next();
      count++;
      try {
        byteIter.remove();
        assertWithMessage("expected exception").fail();
      } catch (UnsupportedOperationException e) {
        // expected
      }
    }
    assertThat(count).isEqualTo(3);
  }

  @Test
  public void testListIterator() {
    LazyStringArrayList rawList = createSampleList();
    UnmodifiableLazyStringList list = new UnmodifiableLazyStringList(rawList);

    ListIterator<String> iter = list.listIterator();
    int count = 0;
    while (iter.hasNext()) {
      iter.next();
      count++;
      try {
        iter.remove();
        assertWithMessage("expected exception").fail();
      } catch (UnsupportedOperationException e) {
        // expected
      }
      try {
        iter.set("bar");
        assertWithMessage("expected exception").fail();
      } catch (UnsupportedOperationException e) {
        // expected
      }
      try {
        iter.add("bar");
        assertWithMessage("expected exception").fail();
      } catch (UnsupportedOperationException e) {
        // expected
      }
    }
    assertThat(count).isEqualTo(3);

    List<ByteString> byteStringList = list.asByteStringList();
    ListIterator<ByteString> byteIter = byteStringList.listIterator();
    count = 0;
    while (byteIter.hasNext()) {
      byteIter.next();
      count++;
      try {
        byteIter.remove();
        assertWithMessage("expected exception").fail();
      } catch (UnsupportedOperationException e) {
        // expected
      }
      try {
        byteIter.set(BYTE_STRING_A);
        assertWithMessage("expected exception").fail();
      } catch (UnsupportedOperationException e) {
        // expected
      }
      try {
        byteIter.add(BYTE_STRING_A);
        assertWithMessage("expected exception").fail();
      } catch (UnsupportedOperationException e) {
        // expected
      }
    }
    assertThat(count).isEqualTo(3);
  }

  private LazyStringArrayList createSampleList() {
    LazyStringArrayList rawList = new LazyStringArrayList();
    rawList.add(STRING_A);
    rawList.add(STRING_B);
    rawList.add(STRING_C);
    return rawList;
  }
}
