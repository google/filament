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

package substr

import (
	"strings"
	"testing"
)

func TestFixSubstr(t *testing.T) {
	type test struct {
		body   string
		substr string
		expect string
	}

	for _, test := range []test{
		{
			body:   "abc_def_ghi_jkl_mno",
			substr: "def_XXX_jkl",
			expect: "def_ghi_jkl",
		},
		{
			body:   "abc\ndef\nghi\njkl\nmno",
			substr: "def\nXXX\njkl",
			expect: "def\nghi\njkl",
		},
		{
			body:   "aaaaa12345ccccc",
			substr: "1x345",
			expect: "12345",
		},
		{
			body:   "aaaaa12345ccccc",
			substr: "12x45",
			expect: "12345",
		},
		{
			body:   "aaaaa12345ccccc",
			substr: "123x5",
			expect: "12345",
		},
		{
			body:   "aaaaaaaaaaaaa",
			substr: "bbbbbbbbbbbbb",
			expect: "", // cannot produce a sensible diff
		}, { ///////////////////////////////////////////////////////////////////
			body: `Return{
  {
    ScalarInitializer[not set]{42u}
  }
}
`,
			substr: `Return{
  {
    ScalarInitializer[not set]{42}
  }
}`,
			expect: `Return{
  {
    ScalarInitializer[not set]{42u}
  }
}`,
		}, { ///////////////////////////////////////////////////////////////////
			body: `VariableDeclStatement{
  Variable{
    x_1
    function
    __u32
  }
}
Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{42u}
}
Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{0u}
}
Return{}
`,
			substr: `Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{42}
}
Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{0}
}`,
			expect: `Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{42u}
}
Assignment{
  Identifier[not set]{x_1}
  ScalarInitializer[not set]{0u}
}`,
		}, { ///////////////////////////////////////////////////////////////////
			body: `VariableDeclStatement{
  Variable{
    a
    function
    __bool
    {
      ScalarInitializer[not set]{true}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __bool
    {
      ScalarInitializer[not set]{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __i32
    {
      ScalarInitializer[not set]{-1}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    function
    __u32
    {
      ScalarInitializer[not set]{1u}
    }
  }
}
VariableDeclStatement{
  Variable{
    e
    function
    __f32
    {
      ScalarInitializer[not set]{1.500000}
    }
  }
}
`,
			substr: `VariableDeclStatement{
  Variable{
    a
    function
    __bool
    {
      ScalarInitializer[not set]{true}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __bool
    {
      ScalarInitializer[not set]{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __i32
    {
      ScalarInitializer[not set]{-1}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    function
    __u32
    {
      ScalarInitializer[not set]{1}
    }
  }
}
VariableDeclStatement{
  Variable{
    e
    function
    __f32
    {
      ScalarInitializer[not set]{1.500000}
    }
  }
}
`,
			expect: `VariableDeclStatement{
  Variable{
    a
    function
    __bool
    {
      ScalarInitializer[not set]{true}
    }
  }
}
VariableDeclStatement{
  Variable{
    b
    function
    __bool
    {
      ScalarInitializer[not set]{false}
    }
  }
}
VariableDeclStatement{
  Variable{
    c
    function
    __i32
    {
      ScalarInitializer[not set]{-1}
    }
  }
}
VariableDeclStatement{
  Variable{
    d
    function
    __u32
    {
      ScalarInitializer[not set]{1u}
    }
  }
}
VariableDeclStatement{
  Variable{
    e
    function
    __f32
    {
      ScalarInitializer[not set]{1.500000}
    }
  }
}
`,
		},
	} {
		body := strings.ReplaceAll(test.body, "\n", "␤")
		substr := strings.ReplaceAll(test.substr, "\n", "␤")
		expect := strings.ReplaceAll(test.expect, "\n", `␤`)
		got := strings.ReplaceAll(Fix(test.body, test.substr), "\n", "␤")
		if got != expect {
			t.Errorf("Test failure:\nbody:   '%v'\nsubstr: '%v'\nexpect: '%v'\ngot:    '%v'\n\n", body, substr, expect, got)
		}

	}
}
