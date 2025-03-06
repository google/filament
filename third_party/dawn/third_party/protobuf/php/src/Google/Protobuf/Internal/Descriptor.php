<?php

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

namespace Google\Protobuf\Internal;

class Descriptor
{
    use HasPublicDescriptorTrait;

    private $full_name;
    private $field = [];
    private $json_to_field = [];
    private $name_to_field = [];
    private $index_to_field = [];
    private $nested_type = [];
    private $enum_type = [];
    private $klass;
    private $legacy_klass;
    private $previous_klass;
    private $options;
    private $oneof_decl = [];

    public function __construct()
    {
        $this->public_desc = new \Google\Protobuf\Descriptor($this);
    }

    public function addOneofDecl($oneof)
    {
        $this->oneof_decl[] = $oneof;
    }

    public function getOneofDecl()
    {
        return $this->oneof_decl;
    }

    public function setFullName($full_name)
    {
        $this->full_name = $full_name;
    }

    public function getFullName()
    {
        return $this->full_name;
    }

    public function addField($field)
    {
        $this->field[$field->getNumber()] = $field;
        $this->json_to_field[$field->getJsonName()] = $field;
        $this->name_to_field[$field->getName()] = $field;
        $this->index_to_field[] = $field;
    }

    public function getField()
    {
        return $this->field;
    }

    public function addNestedType($desc)
    {
        $this->nested_type[] = $desc;
    }

    public function getNestedType()
    {
        return $this->nested_type;
    }

    public function addEnumType($desc)
    {
        $this->enum_type[] = $desc;
    }

    public function getEnumType()
    {
        return $this->enum_type;
    }

    public function getFieldByNumber($number)
    {
        if (!isset($this->field[$number])) {
          return NULL;
        } else {
          return $this->field[$number];
        }
    }

    public function getFieldByJsonName($json_name)
    {
        if (!isset($this->json_to_field[$json_name])) {
          return NULL;
        } else {
          return $this->json_to_field[$json_name];
        }
    }

    public function getFieldByName($name)
    {
        if (!isset($this->name_to_field[$name])) {
          return NULL;
        } else {
          return $this->name_to_field[$name];
        }
    }

    public function getFieldByIndex($index)
    {
        if (count($this->index_to_field) <= $index) {
            return NULL;
        } else {
            return $this->index_to_field[$index];
        }
    }

    public function setClass($klass)
    {
        $this->klass = $klass;
    }

    public function getClass()
    {
        return $this->klass;
    }

    public function setLegacyClass($klass)
    {
        $this->legacy_klass = $klass;
    }

    public function getLegacyClass()
    {
        return $this->legacy_klass;
    }

    public function setPreviouslyUnreservedClass($klass)
    {
        $this->previous_klass = $klass;
    }

    public function getPreviouslyUnreservedClass()
    {
        return $this->previous_klass;
    }

    public function setOptions($options)
    {
        $this->options = $options;
    }

    public function getOptions()
    {
        return $this->options;
    }

    public static function buildFromProto($proto, $file_proto, $containing)
    {
        $desc = new Descriptor();

        $message_name_without_package  = "";
        $classname = "";
        $legacy_classname = "";
        $previous_classname = "";
        $fullname = "";
        GPBUtil::getFullClassName(
            $proto,
            $containing,
            $file_proto,
            $message_name_without_package,
            $classname,
            $legacy_classname,
            $fullname,
            $previous_classname);
        $desc->setFullName($fullname);
        $desc->setClass($classname);
        $desc->setLegacyClass($legacy_classname);
        $desc->setPreviouslyUnreservedClass($previous_classname);
        $desc->setOptions($proto->getOptions());

        foreach ($proto->getField() as $field_proto) {
            $desc->addField(FieldDescriptor::buildFromProto($field_proto));
        }

        // Handle nested types.
        foreach ($proto->getNestedType() as $nested_proto) {
            $desc->addNestedType(Descriptor::buildFromProto(
              $nested_proto, $file_proto, $message_name_without_package));
        }

        // Handle nested enum.
        foreach ($proto->getEnumType() as $enum_proto) {
            $desc->addEnumType(EnumDescriptor::buildFromProto(
              $enum_proto, $file_proto, $message_name_without_package));
        }

        // Handle oneof fields.
        $index = 0;
        foreach ($proto->getOneofDecl() as $oneof_proto) {
            $desc->addOneofDecl(
                OneofDescriptor::buildFromProto($oneof_proto, $desc, $index));
            $index++;
        }

        return $desc;
    }
}
