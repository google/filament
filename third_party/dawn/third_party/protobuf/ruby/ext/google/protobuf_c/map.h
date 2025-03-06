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

#ifndef RUBY_PROTOBUF_MAP_H_
#define RUBY_PROTOBUF_MAP_H_

#include <ruby/ruby.h>

#include "protobuf.h"
#include "ruby-upb.h"

// Returns a Ruby wrapper object for the given map, which will be created if
// one does not exist already.
VALUE Map_GetRubyWrapper(upb_Map *map, upb_CType key_type, TypeInfo value_type,
                         VALUE arena);

// Gets the underlying upb_Map for this Ruby map object, which must have
// key/value type that match |field|. If this is not a map or the type doesn't
// match, raises an exception.
const upb_Map *Map_GetUpbMap(VALUE val, const upb_FieldDef *field,
                             upb_Arena *arena);

// Implements #inspect for this map by appending its contents to |b|.
void Map_Inspect(StringBuilder *b, const upb_Map *map, upb_CType key_type,
                 TypeInfo val_type);

// Returns a new Hash object containing the contents of this Map.
VALUE Map_CreateHash(const upb_Map *map, upb_CType key_type, TypeInfo val_info);

// Returns a deep copy of this Map object.
VALUE Map_deep_copy(VALUE obj);

// Ruby class of Google::Protobuf::Map.
extern VALUE cMap;

// Call at startup to register all types in this module.
void Map_register(VALUE module);

#endif  // RUBY_PROTOBUF_MAP_H_
