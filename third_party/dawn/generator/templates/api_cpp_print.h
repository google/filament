// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{% set API = metadata.api.upper() %}
{% set api = API.lower() %}
#ifndef {{API}}_CPP_PRINT_H_
#define {{API}}_CPP_PRINT_H_

#include "{{api}}/{{api}}_cpp.h"

#include <iomanip>
#include <ios>
#include <ostream>
#include <type_traits>

namespace {{metadata.namespace}} {

  {% for type in by_category["enum"] if type.name.get() != "optional bool" %}
      template <typename CharT, typename Traits>
      std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& o, {{as_cppType(type.name)}} value) {
          switch (value) {
            {% for value in type.values if not is_enum_value_proxy(value) %}
              case {{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}:
                o << "{{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}";
                break;
            {% endfor %}
              default:
                o << "{{as_cppType(type.name)}}::" << std::showbase << std::hex << std::setfill('0') << std::setw(4) << static_cast<typename std::underlying_type<{{as_cppType(type.name)}}>::type>(value);
          }
          return o;
      }
  {% endfor %}

  {% for type in by_category["bitmask"] %}
      template <typename CharT, typename Traits>
      std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& o, {{as_cppType(type.name)}} value) {
        o << "{{as_cppType(type.name)}}::";
        if (!static_cast<bool>(value)) {
          {% for value in type.values if value.value == 0 %}
            // 0 is often explicitly declared as None.
            o << "{{as_cppEnum(value.name)}}";
          {% else %}
            o << std::showbase << std::hex << std::setfill('0') << std::setw(4) << 0;
          {% endfor %}
          return o;
        }

        bool moreThanOneBit = !HasZeroOrOneBits(value);
        if (moreThanOneBit) {
          o << "(";
        }

        bool first = true;
        {% for value in type.values if value.value != 0 %}
          if (value & {{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}) {
            if (!first) {
              o << "|";
            }
            first = false;
            o << "{{as_cppEnum(value.name)}}";
            value &= ~{{as_cppType(type.name)}}::{{as_cppEnum(value.name)}};
          }
        {% endfor %}

        if (static_cast<bool>(value)) {
          if (!first) {
            o << "|";
          }
          o << std::showbase << std::hex << std::setfill('0') << std::setw(4) << static_cast<typename std::underlying_type<{{as_cppType(type.name)}}>::type>(value);
        }

        if (moreThanOneBit) {
          o << ")";
        }
        return o;
      }
  {% endfor %}

  template <typename CharT, typename Traits>
  std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& o, StringView value) {
      o << std::string_view(value);
      return o;
  }

}  // namespace {{metadata.namespace}}

#endif // {{API}}_CPP_PRINT_H_
